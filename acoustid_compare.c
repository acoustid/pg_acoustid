/* acoustid_compare.c */

#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "acoustid.h"
#include "acoustid_utils.h"

/* fingerprint matcher settings */
#define ACOUSTID_MAX_BIT_ERROR 2
#define ACOUSTID_MAX_ALIGN_OFFSET 120
#define ACOUSTID_QUERY_START 80
#define ACOUSTID_QUERY_LENGTH 120
#define ACOUSTID_QUERY_BITS 28
#define ACOUSTID_QUERY_MASK (((1<<ACOUSTID_QUERY_BITS)-1)<<(32-ACOUSTID_QUERY_BITS))
#define ACOUSTID_QUERY_STRIP(x) ((x) & ACOUSTID_QUERY_MASK)

#define MATCH_BITS 14
#define MATCH_MASK ((1 << MATCH_BITS) - 1)
#define MATCH_STRIP(x) ((uint32_t)(x) >> (32 - MATCH_BITS))

#define UNIQ_BITS 16
#define UNIQ_MASK ((1 << MATCH_BITS) - 1)
#define UNIQ_STRIP(x) ((uint32_t)(x) >> (32 - MATCH_BITS))

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(acoustid_compare);
Datum       acoustid_compare(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_compare2);
Datum       acoustid_compare2(PG_FUNCTION_ARGS);

/* From http://en.wikipedia.org/wiki/Hamming_weight */

inline static int
popcount_32(uint32_t x)
{
    const uint32_t m1  = 0x55555555U; /* binary: 0101... */
    const uint32_t m2  = 0x33333333U; /* binary: 00110011.. */
    const uint32_t m4  = 0x0f0f0f0fU; /* binary:  4 zeros,  4 ones ... */
    const uint32_t h01 = 0x01010101U; /* the sum of 256 to the power of 0,1,2,3... */

	x -= (x >> 1) & m1;             /* put count of each 2 bits into those 2 bits */
	x = (x & m2) + ((x >> 2) & m2); /* put count of each 4 bits into those 4 bits */
	x = (x + (x >> 4)) & m4;        /* put count of each 8 bits into those 8 bits */
	return (x * h01) >> 24;         /* returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
}

inline static int
popcount_64(uint64_t x)
{
    const uint64_t m1  = 0x5555555555555555ULL; /* binary: 0101... */
    const uint64_t m2  = 0x3333333333333333ULL; /* binary: 00110011.. */
    const uint64_t m4  = 0x0f0f0f0f0f0f0f0fULL; /* binary:  4 zeros,  4 ones ... */
    const uint64_t h01 = 0x0101010101010101ULL; /* the sum of 256 to the power of 0,1,2,3... */

	x -= (x >> 1) & m1;             /* put count of each 2 bits into those 2 bits */
	x = (x & m2) + ((x >> 2) & m2); /* put count of each 4 bits into those 4 bits */
	x = (x + (x >> 4)) & m4;        /* put count of each 8 bits into those 8 bits */
	return (x * h01) >> 56;         /* returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
}

#define BITCOUNT(x)  popcount_32(x)
#define BITCOUNT64(x)  popcount_64(x)


static float4
match_fingerprints(int32 *a, int asize, int32 *b, int bsize)
{
	int i, j, topcount;
	int numcounts = asize + bsize + 1;
	int *counts = palloc0(sizeof(int) * numcounts);

	for (i = 0; i < asize; i++) {
		int jbegin = Max(0, i - ACOUSTID_MAX_ALIGN_OFFSET);
		int jend = Min(bsize, i + ACOUSTID_MAX_ALIGN_OFFSET);
		for (j = jbegin; j < jend; j++) {
			int biterror = BITCOUNT(a[i] ^ b[j]);
			/* ereport(DEBUG5, (errmsg("comparing %d and %d with error %d", i, j, biterror))); */
			if (biterror <= ACOUSTID_MAX_BIT_ERROR) {
				int offset = i - j + bsize;
				counts[offset]++;			
			}
		}
	}

	topcount = 0;
	for (i = 0; i < numcounts; i++) {
		if (counts[i] > topcount) {
			topcount = counts[i];
		}
	}

	pfree(counts);

	return (float4)topcount / Min(asize, bsize);
}

static float4
match_fingerprints2(int32 *a, int asize, int32 *b, int bsize, int maxoffset)
{
	int i, topcount, topoffset, size, biterror, minsize, auniq = 0, buniq = 0;
	int numcounts = asize + bsize + 1;
	unsigned short *counts = palloc0(sizeof(unsigned short) * numcounts);
	uint8_t *seen;
	uint16_t *aoffsets, *boffsets;
	uint64_t *adata, *bdata;
	float4 score, diversity;

	aoffsets = palloc0(sizeof(uint16_t) * MATCH_MASK * 2);
	boffsets = aoffsets + MATCH_MASK;
	seen = (uint8_t *)aoffsets;

	for (i = 0; i < asize; i++) {
		aoffsets[MATCH_STRIP(a[i])] = i;
	}
	for (i = 0; i < bsize; i++) {
		boffsets[MATCH_STRIP(b[i])] = i;
	}

	topcount = 0;
	topoffset = 0;
	for (i = 0; i < MATCH_MASK; i++) {
		if (aoffsets[i] && boffsets[i]) {
			int offset = aoffsets[i] - boffsets[i];
			if (maxoffset == 0 || (-maxoffset <= offset && offset <= maxoffset)) {
				offset += bsize;
				counts[offset]++;
				if (counts[offset] > topcount) {
					topcount = counts[offset];
					topoffset = offset;
				}
			}
		}
	}

	topoffset -= bsize;

	minsize = Min(asize, bsize) & ~1;
	if (topoffset < 0) {
		b -= topoffset;
		bsize = Max(0, bsize + topoffset);
	}
	else {
		a += topoffset;
		asize = Max(0, asize - topoffset);
	}

	size = Min(asize, bsize) / 2;
	if (!size || !minsize) {
		ereport(DEBUG4, (errmsg("acoustid_compare2: empty matching subfingerprint")));
		score = 0.0;
		goto exit;
	}

	memset(seen, 0, UNIQ_MASK);
	for (i = 0; i < asize; i++) {
		int key = UNIQ_STRIP(a[i]);
		if (!seen[key]) {
			auniq++;
			seen[key] = 1;
		}
	}

	memset(seen, 0, UNIQ_MASK);
	for (i = 0; i < bsize; i++) {
		int key = UNIQ_STRIP(b[i]);
		if (!seen[key]) {
			buniq++;
			seen[key] = 1;
		}
	}

	diversity = Min(Min(1.0, (float)(auniq + 10) / asize + 0.5),
	                Min(1.0, (float)(buniq + 10) / bsize + 0.5));

	ereport(DEBUG5, (errmsg("acoustid_compare2: offset %d, offset score %d, size %d, uniq size %d, diversity %f", topoffset, topcount, size * 2, Max(auniq, buniq), diversity)));

	if (topcount < Max(auniq, buniq) * 0.02) {
		ereport(DEBUG4, (errmsg("acoustid_compare2: top offset score is below 2%% of the unique size")));
		score = 0.0;
		goto exit;
	}

	adata = (uint64_t *)a;
	bdata = (uint64_t *)b;
	biterror = 0;
	for (i = 0; i < size; i++, adata++, bdata++) {
		biterror += BITCOUNT64(*adata ^ *bdata);
	}
	score = (size * 2.0 / minsize) * (1.0 - 2.0 * (float4)biterror / (64 * size));
	if (score < 0.0) {
		score = 0.0;
	}
	if (diversity < 1.0) {
		float newscore = pow(score, 8.0 - 7.0 * diversity);
		ereport(DEBUG4, (errmsg("acoustid_compare2: scaling score because of duplicate items, %f => %f", score, newscore)));
		score = newscore;
	}
exit:
	pfree(aoffsets);
	pfree(counts);
	return score;
}

/* PostgreSQL functions */

Datum
acoustid_compare(PG_FUNCTION_ARGS)
{
	ArrayType *a = PG_GETARG_ARRAYTYPE_P(0);
	ArrayType *b = PG_GETARG_ARRAYTYPE_P(1);
	float4 result;

	CHECKARRVALID(a);
	CHECKARRVALID(b);
	if (ARRISVOID(a) || ARRISVOID(b))
		PG_RETURN_FLOAT4(0.0f);

	result = match_fingerprints(
		ARRPTR(a), ARRNELEMS(a),
		ARRPTR(b), ARRNELEMS(b));

	PG_RETURN_FLOAT4(result);
}

Datum
acoustid_compare2(PG_FUNCTION_ARGS)
{
	ArrayType *a = PG_GETARG_ARRAYTYPE_P(0);
	ArrayType *b = PG_GETARG_ARRAYTYPE_P(1);
	int32 maxoffset = PG_GETARG_INT32(2);
	float4 result;

	CHECKARRVALID(a);
	CHECKARRVALID(b);
	if (ARRISVOID(a) || ARRISVOID(b))
		PG_RETURN_FLOAT4(0.0f);

	result = match_fingerprints2(
		ARRPTR(a), ARRNELEMS(a),
		ARRPTR(b), ARRNELEMS(b),
		maxoffset);

	PG_RETURN_FLOAT4(result);
}

