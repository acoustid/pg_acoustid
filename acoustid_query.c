/* acoustid_query.c */

#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "acoustid_utils.h"
#include "acoustid.h"

#define ACOUSTID_MAX_BIT_ERROR 2
#define ACOUSTID_MAX_ALIGN_OFFSET 120
#define ACOUSTID_QUERY_START 80
#define ACOUSTID_QUERY_LENGTH 120
#define ACOUSTID_QUERY_BITS 28
#define ACOUSTID_QUERY_MASK (((1<<ACOUSTID_QUERY_BITS)-1)<<(32-ACOUSTID_QUERY_BITS))
#define ACOUSTID_QUERY_STRIP(x) ((x) & ACOUSTID_QUERY_MASK)

static inline int
uint32_cmp(const void *a, const void *b)
{
	uint32 aval = *((const uint32 *) a);
	uint32 bval = *((const uint32 *) b);

	if (aval < bval)
		return -1;
	if (aval > bval)
		return 1;
	return 0;
}

PG_FUNCTION_INFO_V1(acoustid_extract_query);
Datum acoustid_extract_query(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_extract_full_query);
Datum acoustid_extract_full_query(PG_FUNCTION_ARGS);

Datum
acoustid_extract_query(PG_FUNCTION_ARGS)
{
	ArrayType *a = PG_GETARG_ARRAYTYPE_P(0), *q;
	int32 *orig, *query;
	int i, j, size, cleansize, querysize;

	CHECKARRVALID(a);
	size = ARRNELEMS(a);
	orig = ARRPTR(a);

	cleansize = 0;
	for (i = 0; i < size; i++) {
		if (orig[i] != 627964279) {
			cleansize++;
		}
	}

	if (cleansize <= 0) {
		PG_RETURN_ARRAYTYPE_P(new_intArrayType(0));
	}

	q = new_intArrayType(120);
	query = ARRPTR(q);
	querysize = 0;
	for (i = Max(0, Min(cleansize - ACOUSTID_QUERY_LENGTH, ACOUSTID_QUERY_START)); i < size && querysize < ACOUSTID_QUERY_LENGTH; i++) {
		int32 x = ACOUSTID_QUERY_STRIP(orig[i]);
		if (orig[i] == 627964279) {
			goto next; // silence
		}
		for (j = 0; j < querysize; j++) { // XXX O(N^2) dupe detection, try if O(N*logN) sorting works better on the tiny array
			if (query[j] == x) {
				goto next; // duplicate
			}
		}
		query[querysize++] = x;
	next: ;
	}
	ARR_DIMS(q)[0] = querysize;

	PG_RETURN_ARRAYTYPE_P(q);
}

Datum
acoustid_extract_full_query(PG_FUNCTION_ARGS)
{
	ArrayType *a, *q;
	uint32 *data, *query_data, mask, value;
	int i, j, size, query_size, bits, shift;

	a = PG_GETARG_ARRAYTYPE_P(0);
	CHECKARRVALID(a);
	data = (uint32 *) ARRPTR(a);
	size = ARRNELEMS(a);

    bits = ACOUSTID_QUERY_BITS;
    if (PG_NARGS() > 1) {
        bits = PG_GETARG_INT32(1);
    }

	q = new_intArrayType(size);
	query_data = (uint32 *) ARRPTR(q);

    shift = 32 - bits;
    mask = (1 << bits) - 1;

	for (i = 0, j = 0; i < size; i++) {
		if (data[i] != SILENCE_HASH) {
            query_data[j++] = (data[i] >> shift) & mask;
		}
	}
    query_size = j;

    qsort(query_data, query_size, sizeof(uint32), uint32_cmp);

    value = UINT32_MAX;
	for (i = 0, j = 0; i < query_size; i++) {
        if (query_data[i] != value) {
            value = query_data[i];
            query_data[j++] = value;
        }
	}
    query_size = j;
    
	ARR_DIMS(q)[0] = query_size;

	PG_RETURN_ARRAYTYPE_P(q);
}

