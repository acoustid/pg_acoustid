/* test_fingerprint_codec.c */

/*
 * Tests for fingerprint_codec.c, outside of PostgreSQL. Everything is
 * checked against reference implementations that read and write the format
 * a bit at a time. Buffers are allocated at exactly the size the codec asks
 * for and filled with junk, so that with AddressSanitizer any access past
 * their end fails, and reading what was never written shows.
 *
 *     make check-codec
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../fingerprint_codec.h"
#include "real_fingerprints.h"

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
			exit(1); \
		} \
	} while (0)

#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
	uint8_t *data;
	size_t len;
} bytes;

static void *xmalloc(size_t len)
{
	/* malloc(0) may return NULL; one byte keeps the pointer valid. */
	void *p = malloc(len ? len : 1);
	CHECK(p != NULL);
	return p;
}

static void *junk(size_t len, uint8_t fill)
{
	void *p = xmalloc(len);
	memset(p, fill, len);
	return p;
}

/* xorshift64* */
static uint64_t rng_state = 1;

static uint64_t rng_next(void)
{
	rng_state ^= rng_state >> 12;
	rng_state ^= rng_state << 25;
	rng_state ^= rng_state >> 27;
	return rng_state * 0x2545F4914F6CDD1DULL;
}

static size_t rng_below(size_t n)
{
	return (size_t) (rng_next() % n);
}

static uint32_t rng_u32(void)
{
	return (uint32_t) (rng_next() >> 32);
}

/* ---------------------------------------------------------------- reference */

static void write_bits(uint8_t *buf, size_t start, unsigned width, unsigned value)
{
	unsigned b;
	for (b = 0; b < width; b++) {
		const size_t bit = start + b;
		buf[bit / 8] |= (uint8_t) (((value >> b) & 1) << (bit % 8));
	}
}

static int read_bits(const uint8_t *buf, size_t len, size_t start, unsigned width, unsigned *value)
{
	unsigned b;
	if (start + width > len * 8) {
		return 0;
	}
	*value = 0;
	for (b = 0; b < width; b++) {
		const size_t bit = start + b;
		*value |= ((unsigned) (buf[bit / 8] >> (bit % 8)) & 1) << b;
	}
	return 1;
}

/* Writes out the given 3-bit and 5-bit values as they are, valid or not. */
static bytes raw_encode(uint8_t version, size_t num_hashes, const uint8_t *normal, size_t num_normal,
                        const uint8_t *exceptions, size_t num_exceptions)
{
	const size_t normal_size = (num_normal * 3 + 7) / 8;
	bytes out;
	size_t i;
	out.len = FPCODEC_HEADER_SIZE + normal_size + (num_exceptions * 5 + 7) / 8;
	out.data = junk(out.len, 0);
	out.data[0] = version;
	out.data[1] = (uint8_t) (num_hashes >> 16);
	out.data[2] = (uint8_t) (num_hashes >> 8);
	out.data[3] = (uint8_t) num_hashes;
	for (i = 0; i < num_normal; i++) {
		write_bits(out.data + FPCODEC_HEADER_SIZE, i * 3, 3, normal[i]);
	}
	for (i = 0; i < num_exceptions; i++) {
		write_bits(out.data + FPCODEC_HEADER_SIZE + normal_size, i * 5, 5, exceptions[i]);
	}
	return out;
}

/* The format written out a bit at a time. */
static bytes reference_encode(uint8_t version, const uint32_t *hashes, size_t n)
{
	uint8_t *normal = xmalloc(n * 33), *exceptions = xmalloc(n * 4);
	size_t num_normal = 0, num_exceptions = 0, i;
	uint32_t prev = 0;
	bytes out;
	for (i = 0; i < n; i++) {
		const uint32_t v = hashes[i] ^ prev;
		unsigned bit, last_bit = 0;
		prev = hashes[i];
		for (bit = 1; bit <= 32; bit++) {
			unsigned delta;
			if (!(v & ((uint32_t) 1 << (bit - 1)))) {
				continue;
			}
			delta = bit - last_bit;
			last_bit = bit;
			if (delta >= 7) {
				normal[num_normal++] = 7;
				exceptions[num_exceptions++] = (uint8_t) (delta - 7);
			} else {
				normal[num_normal++] = (uint8_t) delta;
			}
		}
		normal[num_normal++] = 0;
	}
	out = raw_encode(version, n, normal, num_normal, exceptions, num_exceptions);
	free(normal);
	free(exceptions);
	return out;
}

/*
 * The format read a bit at a time. Returns the number of hashes and sets
 * `*hashes`, or -1 for anything the codec has to reject.
 */
static long reference_decode(const uint8_t *data, size_t len, uint32_t **hashes)
{
	const uint8_t *body = data + FPCODEC_HEADER_SIZE;
	size_t num_hashes, body_len, num_normal = 0, cap = 64, ends = 0, h = 0, exception_bit, i;
	uint8_t *normal;
	uint32_t hash = 0, last_bit = 0;

	*hashes = NULL;
	if (len < FPCODEC_HEADER_SIZE) {
		return -1;
	}
	num_hashes = (size_t) data[1] << 16 | (size_t) data[2] << 8 | data[3];
	if (num_hashes == 0) {
		*hashes = xmalloc(0);
		return 0;
	}
	body_len = len - FPCODEC_HEADER_SIZE;

	normal = xmalloc(cap);
	while (ends < num_hashes) {
		unsigned value;
		if (!read_bits(body, body_len, num_normal * 3, 3, &value)) {
			free(normal);
			return -1;
		}
		if (num_normal == cap) {
			cap *= 2;
			normal = realloc(normal, cap);
			CHECK(normal != NULL);
		}
		normal[num_normal++] = (uint8_t) value;
		if (value == 0) {
			ends++;
		}
	}

	*hashes = xmalloc(num_hashes * sizeof(uint32_t));
	exception_bit = (num_normal * 3 + 7) / 8 * 8;
	for (i = 0; i < num_normal; i++) {
		unsigned delta = normal[i];
		if (delta == 0) {
			(*hashes)[h++] = hash;
			last_bit = 0;
			continue;
		}
		if (delta == 7) {
			unsigned extra;
			if (!read_bits(body, body_len, exception_bit, 5, &extra)) {
				goto invalid;
			}
			delta += extra;
			exception_bit += 5;
		}
		last_bit += delta;
		if (last_bit > 32) {
			goto invalid;
		}
		hash ^= (uint32_t) 1 << (last_bit - 1);
	}
	free(normal);
	return (long) num_hashes;

invalid:
	free(normal);
	free(*hashes);
	*hashes = NULL;
	return -1;
}

/* ---------------------------------------------------------------- codec */

/* Encodes into an output of exactly the encoded size, with junk scratch. */
static bytes codec_encode(uint8_t version, const uint32_t *hashes, size_t n, uint8_t fill)
{
	const size_t scratch_len = fpcodec_encode_scratch_size(n);
	const size_t max_len = fpcodec_max_encoded_size(n);
	uint8_t *scratch = junk(scratch_len, fill), *out = junk(max_len, fill);
	size_t size = 0;
	bytes result;

	CHECK(fpcodec_encode(version, hashes, n, out, max_len, &size, scratch, scratch_len) == FPCODEC_OK);
	CHECK(size <= max_len);
	result.len = size;
	result.data = xmalloc(size);
	memcpy(result.data, out, size);
	free(out);

	/* The same into an output of exactly that size. */
	out = junk(size, fill);
	memset(scratch, fill, scratch_len);
	CHECK(fpcodec_encode(version, hashes, n, out, size, &size, scratch, scratch_len) == FPCODEC_OK);
	CHECK(size == result.len && memcmp(out, result.data, size) == 0);
	free(out);
	free(scratch);
	return result;
}

/*
 * Decodes a copy of the input with output and scratch exactly as large as
 * they need to be and filled with `fill`. Returns like reference_decode.
 */
static long codec_decode(const uint8_t *data, size_t len, uint8_t fill, uint32_t **hashes)
{
	uint8_t *input = xmalloc(len), *scratch, version;
	size_t num_hashes, scratch_len;
	fpcodec_status status;

	*hashes = NULL;
	memcpy(input, data, len);
	status = fpcodec_decode_header(input, len, &version, &num_hashes);
	if (status != FPCODEC_OK) {
		CHECK(status == FPCODEC_INVALID);
		free(input);
		return -1;
	}
	scratch_len = fpcodec_decode_scratch_size(len, num_hashes);
	scratch = junk(scratch_len, fill);
	*hashes = junk(num_hashes * sizeof(uint32_t), fill);
	status = fpcodec_decode(input, len, *hashes, num_hashes, scratch, scratch_len);
	free(scratch);
	free(input);
	if (status != FPCODEC_OK) {
		CHECK(status == FPCODEC_INVALID);
		free(*hashes);
		*hashes = NULL;
		return -1;
	}
	CHECK(version == data[0]);
	return (long) num_hashes;
}

/* Every fill either rejects the input like the reference, or decodes it the same. */
static void expect_decode_matches_reference(const uint8_t *data, size_t len)
{
	static const uint8_t fills[] = { 0x00, 0xFF, 0xA5 };
	uint32_t *expected, *decoded;
	const long n = reference_decode(data, len, &expected);
	size_t f;
	for (f = 0; f < NELEMS(fills); f++) {
		const long m = codec_decode(data, len, fills[f], &decoded);
		CHECK(m == n);
		if (n > 0) {
			CHECK(memcmp(decoded, expected, (size_t) n * sizeof(uint32_t)) == 0);
		}
		free(decoded);
	}
	free(expected);
}

/* Encodes like the reference, and decodes back to the same hashes. */
static void expect_round_trip(uint8_t version, const uint32_t *hashes, size_t n)
{
	bytes expected = reference_encode(version, hashes, n), encoded;
	uint32_t *decoded;
	long m;

	encoded = codec_encode(version, hashes, n, 0xA5);
	CHECK(encoded.len == expected.len && memcmp(encoded.data, expected.data, expected.len) == 0);
	free(encoded.data);
	encoded = codec_encode(version, hashes, n, 0x00);
	CHECK(encoded.len == expected.len && memcmp(encoded.data, expected.data, expected.len) == 0);

	m = codec_decode(encoded.data, encoded.len, 0xA5, &decoded);
	CHECK(m == (long) n);
	CHECK(n == 0 || memcmp(decoded, hashes, n * sizeof(uint32_t)) == 0);
	free(decoded);
	expect_decode_matches_reference(encoded.data, encoded.len);
	free(encoded.data);
	free(expected.data);
}

static void random_hashes(uint32_t *hashes, size_t n)
{
	/*
	 * Sparse, dense and random bits, and runs of repeated hashes, which are
	 * empty after the XOR and so all terminators.
	 */
	size_t i;
	for (i = 0; i < n; i++) {
		switch (rng_below(5)) {
		case 0: hashes[i] = rng_u32(); break;
		case 1: hashes[i] = rng_u32() & rng_u32() & rng_u32(); break;
		case 2: hashes[i] = rng_u32() | rng_u32(); break;
		case 3: hashes[i] = i > 0 ? hashes[i - 1] ^ ((uint32_t) 1 << rng_below(32)) : 0; break;
		default: hashes[i] = i > 0 ? hashes[i - 1] : 0; break;
		}
	}
}

static bytes base64url_decode(const char *s)
{
	static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	const size_t n = strlen(s);
	bytes out;
	uint32_t acc = 0;
	unsigned bits = 0;
	size_t i;
	out.data = xmalloc(n * 3 / 4 + 1);
	out.len = 0;
	for (i = 0; i < n; i++) {
		const char *p = strchr(alphabet, s[i]);
		CHECK(p != NULL && *p != '\0');
		acc = acc << 6 | (uint32_t) (p - alphabet);
		bits += 6;
		if (bits >= 8) {
			bits -= 8;
			out.data[out.len++] = (uint8_t) (acc >> bits);
		}
	}
	return out;
}

/* ---------------------------------------------------------------- tests */

static void test_real_fingerprints(void)
{
	size_t i;
	for (i = 0; i < NELEMS(real_fingerprints); i++) {
		bytes data = base64url_decode(real_fingerprints[i]), encoded;
		uint32_t *expected, *decoded;
		long n = reference_decode(data.data, data.len, &expected);
		CHECK(n > 0);
		CHECK(data.data[0] == 1);
		CHECK(codec_decode(data.data, data.len, 0xA5, &decoded) == n);
		CHECK(memcmp(decoded, expected, (size_t) n * sizeof(uint32_t)) == 0);

		encoded = codec_encode(1, decoded, (size_t) n, 0xA5);
		CHECK(encoded.len == data.len && memcmp(encoded.data, data.data, data.len) == 0);
		free(encoded.data);
		free(decoded);
		free(expected);
		free(data.data);
	}
}

static void test_edge_cases(void)
{
	static const uint32_t one_bit[] = { 1 }, zero[] = { 0 }, all_bits[] = { 0xFFFFFFFF };
	/* Only the top bit: a distance of 32, the largest exception. */
	static const uint32_t top_bit[] = { 0x80000000, 0, 0x80000000 };
	/* Bits 7, 14, 21 and 28 apart: four exceptions in one hash. */
	static const uint32_t four_exceptions[] = { 0x08102040, 0, 0x08102040, 0x08102040 ^ 0x80000000 };
	/* Distances of exactly 6 and 7, either side of the exception cut-off. */
	static const uint32_t six_seven[] = { 0x00000820, 0x00001040 ^ 0x00000820 };
	/* Bit 32 reached with distances of 6 and 2. */
	static const uint32_t bit_32[] = { 0x80000000 | 0x20820820 };
	static const uint32_t small[] = { 1, 2, 3 };

	expect_round_trip(1, NULL, 0);
	expect_round_trip(1, zero, NELEMS(zero));
	expect_round_trip(1, one_bit, NELEMS(one_bit));
	expect_round_trip(1, all_bits, NELEMS(all_bits));
	expect_round_trip(1, top_bit, NELEMS(top_bit));
	expect_round_trip(1, four_exceptions, NELEMS(four_exceptions));
	expect_round_trip(1, six_seven, NELEMS(six_seven));
	expect_round_trip(1, bit_32, NELEMS(bit_32));
	/* The version is carried through. */
	expect_round_trip(0, small, NELEMS(small));
	expect_round_trip(0xAB, small, NELEMS(small));
}

static void test_long_runs(void)
{
	/*
	 * Repeated hashes make a terminator per value; alternating all-ones
	 * makes 33 values per hash. Lengths around the groups of eight.
	 */
	static const size_t lens[] = { 1, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 100, 500, 501, 1000 };
	uint32_t hashes[1000];
	size_t i;
	for (i = 0; i < NELEMS(hashes); i++) {
		hashes[i] = i < 500 ? 0x12345678 : (i % 2 == 0 ? 0 : 0xFFFFFFFF);
	}
	for (i = 0; i < NELEMS(lens); i++) {
		expect_round_trip(1, hashes, lens[i]);
		expect_round_trip(1, hashes + NELEMS(hashes) - lens[i], lens[i]);
	}
}

static void test_random_round_trips(void)
{
	static uint32_t hashes[3000];
	size_t i;
	for (i = 0; i < 300; i++) {
		const size_t max_len = i % 10 == 0 ? NELEMS(hashes) : 300;
		const size_t len = 1 + rng_below(max_len);
		random_hashes(hashes, len);
		expect_round_trip(1, hashes, len);
	}
}

static void test_truncations(void)
{
	uint32_t hashes[200], *decoded;
	bytes encoded;
	size_t len;
	random_hashes(hashes, NELEMS(hashes));
	encoded = reference_encode(1, hashes, NELEMS(hashes));
	for (len = 0; len < encoded.len; len++) {
		CHECK(reference_decode(encoded.data, len, &decoded) == -1);
		expect_decode_matches_reference(encoded.data, len);
	}
	free(encoded.data);
}

static void test_corrupted(void)
{
	uint32_t hashes[200];
	size_t iter;
	for (iter = 0; iter < 3000; iter++) {
		const size_t len = 1 + rng_below(NELEMS(hashes));
		bytes encoded, data;
		uint8_t *body;
		size_t body_len, i, k;

		random_hashes(hashes, len);
		encoded = reference_encode(1, hashes, len);
		data.len = encoded.len + 48;
		data.data = junk(data.len, 0);
		memcpy(data.data, encoded.data, encoded.len);
		data.len = encoded.len;
		body = data.data + FPCODEC_HEADER_SIZE;
		body_len = encoded.len - FPCODEC_HEADER_SIZE;

		switch (rng_below(6)) {
		case 0:
		case 1:
			/* Flipped bits. */
			for (k = 1 + rng_below(3); k > 0; k--) {
				const size_t bit = rng_below(body_len * 8);
				body[bit / 8] ^= (uint8_t) (1 << (bit % 8));
			}
			break;
		case 2: {
			/* A hash count off by a little. */
			size_t count = (size_t) data.data[1] << 16 | (size_t) data.data[2] << 8 | data.data[3];
			const size_t delta = 1 + rng_below(3);
			count = rng_below(2) ? count + delta : (count > delta ? count - delta : 0);
			data.data[1] = (uint8_t) (count >> 16);
			data.data[2] = (uint8_t) (count >> 8);
			data.data[3] = (uint8_t) count;
			break;
		}
		case 3:
			/* Trailing set bits, which read as 7s and exceptions that are not there. */
			k = rng_below(48);
			memset(data.data + data.len, 0xFF, k);
			data.len += k;
			break;
		case 4:
			/* Random bytes, sparse enough that some of them decode. */
			for (i = 0; i < body_len; i++) {
				body[i] = (uint8_t) (rng_u32() & rng_u32());
			}
			break;
		default:
			/* Terminators with a few 7s among them. */
			memset(body, 0, body_len);
			for (k = rng_below(20); k > 0; k--) {
				write_bits(body, rng_below(body_len * 8 / 3) * 3, 3, 7);
			}
			break;
		}
		expect_decode_matches_reference(data.data, data.len);
		free(data.data);
		free(encoded.data);
	}
}

typedef struct {
	const uint8_t *normal;
	size_t num_normal;
	const uint8_t *exceptions;
	size_t num_exceptions;
} raw_hash;

/* `before` valid hashes, `hash`, and `after` valid hashes. */
static bytes raw_stream(size_t before, const raw_hash *hash, size_t after, const raw_hash *fillers, size_t num_fillers)
{
	const size_t total = before + 1 + after;
	uint8_t *normal = xmalloc(total * 8), *exceptions = xmalloc(total * 4);
	size_t num_normal = 0, num_exceptions = 0, i;
	bytes out;
	for (i = 0; i < total; i++) {
		const raw_hash *h = i == before ? hash : &fillers[i % num_fillers];
		memcpy(normal + num_normal, h->normal, h->num_normal);
		num_normal += h->num_normal;
		if (h->num_exceptions > 0) {
			memcpy(exceptions + num_exceptions, h->exceptions, h->num_exceptions);
			num_exceptions += h->num_exceptions;
		}
	}
	out = raw_encode(1, total, normal, num_normal, exceptions, num_exceptions);
	free(normal);
	free(exceptions);
	return out;
}

static void test_broken_hashes(void)
{
	static const uint8_t one_bit[] = { 1, 0 }, seven[] = { 7, 0 }, three[] = { 3 }, twenty_six[] = { 26 };
	static const uint8_t six_sixes[] = { 6, 6, 6, 6, 6, 6, 0 }, past_32[] = { 6, 6, 6, 6, 6, 1, 2, 0 };
	/* Valid hashes around a broken one: one bit, and one bit ten up, an exception. */
	static const raw_hash fillers[] = {
		{ one_bit, 2, NULL, 0 },
		{ seven, 2, three, 1 },
	};
	static const struct {
		raw_hash hash;
		size_t num_fillers;
	} cases[] = {
		/* Six distances of 6 reach 36. */
		{ { six_sixes, 7, NULL, 0 }, 2 },
		/* 7 + 26 reaches 33. */
		{ { seven, 2, twenty_six, 1 }, 2 },
		/* 6 * 5 + 1 + 2 reaches 33. */
		{ { past_32, 8, NULL, 0 }, 2 },
		/* A 7 with no exception left for it. */
		{ { seven, 2, NULL, 0 }, 1 },
	};
	/* In the batches of eight, at their edges, and at the end. */
	static const size_t counts[] = { 0, 1, 3, 4, 7, 8, 9, 15, 16, 17, 33, 100 };
	size_t c, b, a;
	for (c = 0; c < NELEMS(cases); c++) {
		for (b = 0; b < NELEMS(counts); b++) {
			for (a = 0; a < NELEMS(counts); a++) {
				uint32_t *decoded;
				bytes data = raw_stream(counts[b], &cases[c].hash, counts[a], fillers, cases[c].num_fillers);
				bytes valid = raw_stream(counts[b], &fillers[0], counts[a], fillers, cases[c].num_fillers);
				CHECK(reference_decode(data.data, data.len, &decoded) == -1);
				expect_decode_matches_reference(data.data, data.len);
				/* The same stream with a valid hash in its place decodes. */
				CHECK(reference_decode(valid.data, valid.len, &decoded) == (long) (counts[b] + 1 + counts[a]));
				free(decoded);
				expect_decode_matches_reference(valid.data, valid.len);
				free(data.data);
				free(valid.data);
			}
		}
	}
}

static void test_runs_out_of_terminators(void)
{
	static const uint8_t one_bit[] = { 1, 0 };
	static const raw_hash fillers[] = { { one_bit, 2, NULL, 0 } };
	static const size_t counts[] = { 1, 8, 9, 32, 33, 100 };
	size_t i;
	for (i = 0; i < NELEMS(counts); i++) {
		uint32_t *decoded;
		bytes valid = raw_stream(0, &fillers[0], counts[i] - 1, fillers, 1);
		/* One hash more than there are, and room for it, but only 7s. */
		uint8_t *data = junk(valid.len + 8, 0xFF);
		memcpy(data, valid.data, valid.len);
		data[1] = (uint8_t) ((counts[i] + 1) >> 16);
		data[2] = (uint8_t) ((counts[i] + 1) >> 8);
		data[3] = (uint8_t) (counts[i] + 1);
		CHECK(reference_decode(data, valid.len + 8, &decoded) == -1);
		expect_decode_matches_reference(data, valid.len + 8);
		free(data);
		free(valid.data);
	}
}

static void test_header(void)
{
	static const uint8_t short_input[] = { 1, 0, 0 };
	static const uint8_t no_body[] = { 1, 0, 0, 1 };
	static const uint8_t huge_count[] = { 1, 0xFF, 0xFF, 0xFF };
	/* Three bytes hold eight terminators, but not nine. */
	static const uint8_t eight[] = { 1, 0, 0, 8, 0, 0, 0 }, nine[] = { 1, 0, 0, 9, 0, 0, 0 };
	/* No hashes: an empty fingerprint, whatever follows. */
	static const uint8_t empty[] = { 1, 0, 0, 0 }, empty_trailing[] = { 1, 0, 0, 0, 0xFF, 0xB6, 0x6D, 0x03 };
	uint8_t version;
	size_t n;
	uint32_t *decoded;
	size_t i;

	CHECK(fpcodec_decode_header(short_input, sizeof(short_input), &version, &n) == FPCODEC_INVALID);
	CHECK(fpcodec_decode_header(no_body, sizeof(no_body), &version, &n) == FPCODEC_INVALID);
	CHECK(fpcodec_decode_header(huge_count, sizeof(huge_count), &version, &n) == FPCODEC_INVALID);
	CHECK(fpcodec_decode_header(nine, sizeof(nine), &version, &n) == FPCODEC_INVALID);
	CHECK(fpcodec_decode_header(eight, sizeof(eight), &version, &n) == FPCODEC_OK && n == 8 && version == 1);
	CHECK(codec_decode(eight, sizeof(eight), 0xA5, &decoded) == 8);
	for (i = 0; i < 8; i++) {
		CHECK(decoded[i] == 0);
	}
	free(decoded);

	CHECK(codec_decode(empty, sizeof(empty), 0xA5, &decoded) == 0);
	free(decoded);
	CHECK(codec_decode(empty_trailing, sizeof(empty_trailing), 0xA5, &decoded) == 0);
	free(decoded);
	expect_decode_matches_reference(empty_trailing, sizeof(empty_trailing));
}

static void test_short_inputs(void)
{
	/*
	 * Every body of up to two bytes, with every hash count the header
	 * accepts for it. Tiny inputs leave the least room in scratch for reads
	 * past the end of the streams.
	 */
	uint8_t data[FPCODEC_HEADER_SIZE + 2] = { 1, 0, 0, 0 };
	size_t body_len, count;
	uint32_t body;
	for (body_len = 0; body_len <= 2; body_len++) {
		for (body = 0; body < (1u << (8 * body_len)); body++) {
			data[4] = (uint8_t) body;
			data[5] = (uint8_t) (body >> 8);
			for (count = 0; count <= body_len * 8 / 3 + 1; count++) {
				data[3] = (uint8_t) count;
				expect_decode_matches_reference(data, FPCODEC_HEADER_SIZE + body_len);
			}
		}
	}
}

static void test_trailing_bytes(void)
{
	static const uint32_t hashes[] = { 1, 2, 3 };
	bytes encoded = reference_encode(1, hashes, NELEMS(hashes));
	uint8_t *padded = xmalloc(encoded.len + 7);
	uint32_t *decoded;
	memcpy(padded, encoded.data, encoded.len);
	memcpy(padded + encoded.len, "garbage", 7);
	CHECK(codec_decode(padded, encoded.len + 7, 0xA5, &decoded) == 3);
	CHECK(memcmp(decoded, hashes, sizeof(hashes)) == 0);
	free(decoded);
	free(padded);
	free(encoded.data);
}

static void test_large_trailing_data(void)
{
	/*
	 * A megabyte after a small fingerprint is ignored, and does not make
	 * the scratch any larger than for the most a fingerprint of that many
	 * hashes can take.
	 */
	static const uint32_t hashes[] = { 0xDEADBEEF, 0x12345678, 0xCAFEBABE };
	const size_t n = NELEMS(hashes), trailing = 1 << 20;
	bytes encoded = reference_encode(1, hashes, n);
	uint8_t *data = junk(encoded.len + trailing, 0xFF);
	uint32_t *decoded;
	memcpy(data, encoded.data, encoded.len);
	CHECK(fpcodec_decode_scratch_size(encoded.len + trailing, n) == fpcodec_decode_scratch_size(fpcodec_max_encoded_size(n), n));
	CHECK(fpcodec_decode_scratch_size(encoded.len + trailing, n) < 64 * n + 64);
	CHECK(codec_decode(data, encoded.len + trailing, 0xA5, &decoded) == (long) n);
	CHECK(memcmp(decoded, hashes, sizeof(hashes)) == 0);
	free(decoded);
	free(data);
	free(encoded.data);
}

static void test_size_limits(void)
{
	/* The largest sizes do not overflow even with a 32-bit size_t. */
	CHECK(fpcodec_max_encoded_size(FPCODEC_MAX_HASHES) > FPCODEC_MAX_HASHES);
	CHECK(fpcodec_encode_scratch_size(FPCODEC_MAX_HASHES) > (size_t) FPCODEC_MAX_HASHES * 33);
	CHECK(fpcodec_decode_scratch_size((size_t) -1, FPCODEC_MAX_HASHES) > fpcodec_max_encoded_size(FPCODEC_MAX_HASHES));
	CHECK(fpcodec_decode_scratch_size((size_t) -1, FPCODEC_MAX_HASHES) < (size_t) 1 << 30);
	/* Past the 24-bit hash count there is nothing to size. */
	CHECK(fpcodec_max_encoded_size(FPCODEC_MAX_HASHES + 1) == 0);
	CHECK(fpcodec_encode_scratch_size(FPCODEC_MAX_HASHES + 1) == 0);
	CHECK(fpcodec_decode_scratch_size(1000, FPCODEC_MAX_HASHES + 1) == 0);
	CHECK(fpcodec_max_encoded_size((size_t) -1) == 0);
}

static void test_buffers_too_small(void)
{
	static const uint32_t hashes[] = { 0xDEADBEEF, 0x12345678, 0xCAFEBABE, 0x08102040 };
	const size_t n = NELEMS(hashes);
	const size_t scratch_len = fpcodec_encode_scratch_size(n);
	uint8_t *scratch = junk(scratch_len, 0xA5), version;
	uint8_t out[64];
	uint32_t decoded[NELEMS(hashes)];
	uint8_t *decode_scratch;
	size_t size, len, num_hashes;

	CHECK(fpcodec_max_encoded_size(n) <= sizeof(out));
	CHECK(fpcodec_encode(1, hashes, n, out, sizeof(out), &size, scratch, scratch_len) == FPCODEC_OK);
	for (len = 0; len < size; len++) {
		size_t ignored;
		CHECK(fpcodec_encode(1, hashes, n, out, len, &ignored, scratch, scratch_len) == FPCODEC_BUFFER_TOO_SMALL);
	}
	CHECK(fpcodec_encode(1, hashes, n, out, size, &size, scratch, scratch_len - 1) == FPCODEC_BUFFER_TOO_SMALL);
	CHECK(fpcodec_encode(1, hashes, n, out, size, &size, scratch, scratch_len) == FPCODEC_OK);
	/* The size in the header is 24 bits; nothing is read before that is checked. */
	CHECK(fpcodec_encode(1, NULL, FPCODEC_MAX_HASHES + 1, out, sizeof(out), &size, scratch, scratch_len) == FPCODEC_INVALID);
	CHECK(fpcodec_encode(1, hashes, n, out, sizeof(out), &size, scratch, scratch_len) == FPCODEC_OK);

	CHECK(fpcodec_decode_header(out, size, &version, &num_hashes) == FPCODEC_OK && num_hashes == n);
	decode_scratch = junk(fpcodec_decode_scratch_size(size, n), 0xA5);
	CHECK(fpcodec_decode(out, size, decoded, n - 1, decode_scratch, fpcodec_decode_scratch_size(size, n)) == FPCODEC_BUFFER_TOO_SMALL);
	CHECK(fpcodec_decode(out, size, decoded, n, decode_scratch, fpcodec_decode_scratch_size(size, n) - 1) == FPCODEC_BUFFER_TOO_SMALL);
	CHECK(fpcodec_decode(out, size, decoded, n, decode_scratch, fpcodec_decode_scratch_size(size, n)) == FPCODEC_OK);
	CHECK(memcmp(decoded, hashes, sizeof(hashes)) == 0);
	free(decode_scratch);
	free(scratch);
}

int main(void)
{
	static const struct {
		const char *name;
		void (*run)(void);
	} tests[] = {
		{ "real fingerprints", test_real_fingerprints },
		{ "edge cases", test_edge_cases },
		{ "long runs", test_long_runs },
		{ "random round trips", test_random_round_trips },
		{ "truncations", test_truncations },
		{ "corrupted input", test_corrupted },
		{ "broken hashes", test_broken_hashes },
		{ "runs out of terminators", test_runs_out_of_terminators },
		{ "header", test_header },
		{ "short inputs", test_short_inputs },
		{ "trailing bytes", test_trailing_bytes },
		{ "large trailing data", test_large_trailing_data },
		{ "size limits", test_size_limits },
		{ "buffers too small", test_buffers_too_small },
	};
	size_t i;
	for (i = 0; i < NELEMS(tests); i++) {
		tests[i].run();
		printf("ok - %s\n", tests[i].name);
	}
	return 0;
}
