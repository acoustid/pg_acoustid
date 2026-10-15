/* fingerprint_codec.c */

/*
 * Both directions go through a byte per value. Eight 3-bit values are three
 * bytes and eight 5-bit values five, and everything else works on eight
 * values at a time in a 64-bit word, without branching on the values.
 *
 * Words are read and written little endian whatever the machine, so the
 * byte arithmetic is the same everywhere.
 */

#include <string.h>

#include "fingerprint_codec.h"

#define ONES   0x0101010101010101ULL
#define HIGHS  0x8080808080808080ULL
#define LOW7S  0x7F7F7F7F7F7F7F7FULL
#define SEVENS 0x0707070707070707ULL

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_MSC_VER)
#define FPCODEC_LITTLE_ENDIAN 1
#endif

static inline uint64_t load64(const uint8_t *p)
{
#ifdef FPCODEC_LITTLE_ENDIAN
	uint64_t x;
	memcpy(&x, p, 8);
	return x;
#else
	return (uint64_t) p[0] | (uint64_t) p[1] << 8 | (uint64_t) p[2] << 16 | (uint64_t) p[3] << 24 |
	       (uint64_t) p[4] << 32 | (uint64_t) p[5] << 40 | (uint64_t) p[6] << 48 | (uint64_t) p[7] << 56;
#endif
}

static inline void store64(uint8_t *p, uint64_t x)
{
#ifdef FPCODEC_LITTLE_ENDIAN
	memcpy(p, &x, 8);
#else
	int i;
	for (i = 0; i < 8; i++) {
		p[i] = (uint8_t) (x >> (8 * i));
	}
#endif
}

/* The number of bytes in `x` that have only their top bit set. */
static inline unsigned count_high_bits(uint64_t x)
{
	return (unsigned) ((((x >> 7) & ONES) * ONES) >> 56);
}

/* 0x80 in each byte of `x` that is zero, 0 elsewhere. */
static inline uint64_t zero_bytes(uint64_t x)
{
	return ~(((x & LOW7S) + LOW7S) | x | LOW7S);
}

/*
 * For each byte, the distances between its set bits, counting from bit 0
 * as 1, packed a byte each; the number of set bits; and the position of the
 * highest set bit, counting from 1.
 */
static const uint64_t byte_deltas[256] = {
	0x0000000000000000, 0x0000000000000001, 0x0000000000000002, 0x0000000000000101,
	0x0000000000000003, 0x0000000000000201, 0x0000000000000102, 0x0000000000010101,
	0x0000000000000004, 0x0000000000000301, 0x0000000000000202, 0x0000000000020101,
	0x0000000000000103, 0x0000000000010201, 0x0000000000010102, 0x0000000001010101,
	0x0000000000000005, 0x0000000000000401, 0x0000000000000302, 0x0000000000030101,
	0x0000000000000203, 0x0000000000020201, 0x0000000000020102, 0x0000000002010101,
	0x0000000000000104, 0x0000000000010301, 0x0000000000010202, 0x0000000001020101,
	0x0000000000010103, 0x0000000001010201, 0x0000000001010102, 0x0000000101010101,
	0x0000000000000006, 0x0000000000000501, 0x0000000000000402, 0x0000000000040101,
	0x0000000000000303, 0x0000000000030201, 0x0000000000030102, 0x0000000003010101,
	0x0000000000000204, 0x0000000000020301, 0x0000000000020202, 0x0000000002020101,
	0x0000000000020103, 0x0000000002010201, 0x0000000002010102, 0x0000000201010101,
	0x0000000000000105, 0x0000000000010401, 0x0000000000010302, 0x0000000001030101,
	0x0000000000010203, 0x0000000001020201, 0x0000000001020102, 0x0000000102010101,
	0x0000000000010104, 0x0000000001010301, 0x0000000001010202, 0x0000000101020101,
	0x0000000001010103, 0x0000000101010201, 0x0000000101010102, 0x0000010101010101,
	0x0000000000000007, 0x0000000000000601, 0x0000000000000502, 0x0000000000050101,
	0x0000000000000403, 0x0000000000040201, 0x0000000000040102, 0x0000000004010101,
	0x0000000000000304, 0x0000000000030301, 0x0000000000030202, 0x0000000003020101,
	0x0000000000030103, 0x0000000003010201, 0x0000000003010102, 0x0000000301010101,
	0x0000000000000205, 0x0000000000020401, 0x0000000000020302, 0x0000000002030101,
	0x0000000000020203, 0x0000000002020201, 0x0000000002020102, 0x0000000202010101,
	0x0000000000020104, 0x0000000002010301, 0x0000000002010202, 0x0000000201020101,
	0x0000000002010103, 0x0000000201010201, 0x0000000201010102, 0x0000020101010101,
	0x0000000000000106, 0x0000000000010501, 0x0000000000010402, 0x0000000001040101,
	0x0000000000010303, 0x0000000001030201, 0x0000000001030102, 0x0000000103010101,
	0x0000000000010204, 0x0000000001020301, 0x0000000001020202, 0x0000000102020101,
	0x0000000001020103, 0x0000000102010201, 0x0000000102010102, 0x0000010201010101,
	0x0000000000010105, 0x0000000001010401, 0x0000000001010302, 0x0000000101030101,
	0x0000000001010203, 0x0000000101020201, 0x0000000101020102, 0x0000010102010101,
	0x0000000001010104, 0x0000000101010301, 0x0000000101010202, 0x0000010101020101,
	0x0000000101010103, 0x0000010101010201, 0x0000010101010102, 0x0001010101010101,
	0x0000000000000008, 0x0000000000000701, 0x0000000000000602, 0x0000000000060101,
	0x0000000000000503, 0x0000000000050201, 0x0000000000050102, 0x0000000005010101,
	0x0000000000000404, 0x0000000000040301, 0x0000000000040202, 0x0000000004020101,
	0x0000000000040103, 0x0000000004010201, 0x0000000004010102, 0x0000000401010101,
	0x0000000000000305, 0x0000000000030401, 0x0000000000030302, 0x0000000003030101,
	0x0000000000030203, 0x0000000003020201, 0x0000000003020102, 0x0000000302010101,
	0x0000000000030104, 0x0000000003010301, 0x0000000003010202, 0x0000000301020101,
	0x0000000003010103, 0x0000000301010201, 0x0000000301010102, 0x0000030101010101,
	0x0000000000000206, 0x0000000000020501, 0x0000000000020402, 0x0000000002040101,
	0x0000000000020303, 0x0000000002030201, 0x0000000002030102, 0x0000000203010101,
	0x0000000000020204, 0x0000000002020301, 0x0000000002020202, 0x0000000202020101,
	0x0000000002020103, 0x0000000202010201, 0x0000000202010102, 0x0000020201010101,
	0x0000000000020105, 0x0000000002010401, 0x0000000002010302, 0x0000000201030101,
	0x0000000002010203, 0x0000000201020201, 0x0000000201020102, 0x0000020102010101,
	0x0000000002010104, 0x0000000201010301, 0x0000000201010202, 0x0000020101020101,
	0x0000000201010103, 0x0000020101010201, 0x0000020101010102, 0x0002010101010101,
	0x0000000000000107, 0x0000000000010601, 0x0000000000010502, 0x0000000001050101,
	0x0000000000010403, 0x0000000001040201, 0x0000000001040102, 0x0000000104010101,
	0x0000000000010304, 0x0000000001030301, 0x0000000001030202, 0x0000000103020101,
	0x0000000001030103, 0x0000000103010201, 0x0000000103010102, 0x0000010301010101,
	0x0000000000010205, 0x0000000001020401, 0x0000000001020302, 0x0000000102030101,
	0x0000000001020203, 0x0000000102020201, 0x0000000102020102, 0x0000010202010101,
	0x0000000001020104, 0x0000000102010301, 0x0000000102010202, 0x0000010201020101,
	0x0000000102010103, 0x0000010201010201, 0x0000010201010102, 0x0001020101010101,
	0x0000000000010106, 0x0000000001010501, 0x0000000001010402, 0x0000000101040101,
	0x0000000001010303, 0x0000000101030201, 0x0000000101030102, 0x0000010103010101,
	0x0000000001010204, 0x0000000101020301, 0x0000000101020202, 0x0000010102020101,
	0x0000000101020103, 0x0000010102010201, 0x0000010102010102, 0x0001010201010101,
	0x0000000001010105, 0x0000000101010401, 0x0000000101010302, 0x0000010101030101,
	0x0000000101010203, 0x0000010101020201, 0x0000010101020102, 0x0001010102010101,
	0x0000000101010104, 0x0000010101010301, 0x0000010101010202, 0x0001010101020101,
	0x0000010101010103, 0x0001010101010201, 0x0001010101010102, 0x0101010101010101,
};

static const uint8_t byte_bit_count[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

static const uint8_t byte_last_bit[256] = {
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

/*
 * The low three bits of each byte of `x`, concatenated into 24 bits, and
 * the reverse. Written out, as compilers do not always unroll the loop, and
 * shifts by a variable amount are slow.
 */
static inline uint32_t pack3(uint64_t x)
{
	return (uint32_t) ((x & 7) | (x >> 5 & 0x38) | (x >> 10 & 0x1C0) | (x >> 15 & 0xE00) |
	                   (x >> 20 & 0x7000) | (x >> 25 & 0x38000) | (x >> 30 & 0x1C0000) | (x >> 35 & 0xE00000));
}

static inline uint64_t spread3(uint32_t x)
{
	const uint64_t y = x;
	return (y & 7) | (y & 0x38) << 5 | (y & 0x1C0) << 10 | (y & 0xE00) << 15 |
	       (y & 0x7000) << 20 | (y & 0x38000) << 25 | (y & 0x1C0000) << 30 | (y & 0xE00000) << 35;
}

size_t fpcodec_max_encoded_size(size_t num_hashes)
{
	if (num_hashes > FPCODEC_MAX_HASHES) {
		return 0;
	}
	/*
	 * At most 32 set bits and a terminator per hash, and at most four
	 * distances of 7 or more in 32 bits.
	 */
	return FPCODEC_HEADER_SIZE + (num_hashes * 33 * 3 + 7) / 8 + (num_hashes * 4 * 5 + 7) / 8;
}

static size_t encode_values_size(size_t num_hashes)
{
	/* At most 33 values per hash, and an eight-byte write past the last. */
	return num_hashes * 33 + 7;
}

static size_t encode_exceptions_size(size_t num_hashes)
{
	/* At most four exceptions per hash, and an eight-byte write at the end. */
	return num_hashes * 4 * 5 / 8 + 8;
}

size_t fpcodec_encode_scratch_size(size_t num_hashes)
{
	if (num_hashes > FPCODEC_MAX_HASHES) {
		return 0;
	}
	return encode_values_size(num_hashes) + encode_exceptions_size(num_hashes);
}

fpcodec_status fpcodec_encode(uint8_t version, const uint32_t *hashes, size_t num_hashes,
                              uint8_t *out, size_t out_len, size_t *encoded_size,
                              uint8_t *scratch, size_t scratch_len)
{
	uint8_t *values, *exceptions, *normal;
	size_t num_values = 0, padded, normal_size, exceptions_size = 0, size, i, o;
	uint32_t prev = 0;
	uint64_t acc = 0;
	unsigned acc_bits = 0;

	if (num_hashes > FPCODEC_MAX_HASHES) {
		return FPCODEC_INVALID;
	}
	if (scratch_len < fpcodec_encode_scratch_size(num_hashes)) {
		return FPCODEC_BUFFER_TOO_SMALL;
	}
	values = scratch;
	exceptions = scratch + encode_values_size(num_hashes);

	/*
	 * The distances between set bits, uncapped, a byte each. Each byte of a
	 * hash comes from a table, eight bytes written at once and the next
	 * byte's written over the unused part. The first distance is made
	 * relative to the last set bit of the bytes before; adding to the whole
	 * word only changes its lowest byte, as no distance is over 32.
	 */
	for (i = 0; i < num_hashes; i++) {
		const uint32_t x = hashes[i] ^ prev;
		uint32_t last_bit = 0;
		prev = hashes[i];
#define ENCODE_BYTE(k) \
		do { \
			const uint8_t b = (uint8_t) (x >> (8 * (k))); \
			store64(values + num_values, byte_deltas[b] + (8u * (k) - last_bit)); \
			num_values += byte_bit_count[b]; \
			last_bit = b ? 8u * (k) + byte_last_bit[b] : last_bit; \
		} while (0)
		ENCODE_BYTE(0);
		ENCODE_BYTE(1);
		ENCODE_BYTE(2);
		ENCODE_BYTE(3);
#undef ENCODE_BYTE
		values[num_values++] = 0;
	}

	normal_size = (num_values * 3 + 7) / 8;
	if (out_len < FPCODEC_HEADER_SIZE + normal_size) {
		return FPCODEC_BUFFER_TOO_SMALL;
	}
	normal = out + FPCODEC_HEADER_SIZE;

	/*
	 * Eight values at a time: the ones over 6, capped at 7, into the normal
	 * stream, and the rest of them, minus 7, into the exception stream.
	 */
	padded = (num_values + 7) & ~(size_t) 7;
	memset(values + num_values, 0, padded - num_values);
	for (i = 0, o = 0; i < padded; i += 8, o += 3) {
		const uint64_t x = load64(values + i);
		/* 0x80 in each byte that is 7 or more; adding 0x79 never carries. */
		const uint64_t big = (x + 0x7979797979797979ULL) & HIGHS;
		const uint64_t big_bytes = (big >> 7) * 0xFF;
		const uint64_t sevens = SEVENS & big_bytes;
		const uint64_t rest = (x & big_bytes) - sevens;
		const uint64_t capped = (x & ~big_bytes) | sevens;
		const uint32_t packed = pack3(capped);
		uint64_t extra = 0;
		unsigned shift = 0, full_bytes, j;

#define TAKE_EXCEPTION(j) \
		do { \
			const uint64_t selected = (big >> (8 * (j) + 7)) & 1; \
			extra |= ((rest >> (8 * (j))) & 0x1F & (0 - selected)) << shift; \
			shift += 5 * (unsigned) selected; \
		} while (0)
		TAKE_EXCEPTION(0);
		TAKE_EXCEPTION(1);
		TAKE_EXCEPTION(2);
		TAKE_EXCEPTION(3);
		TAKE_EXCEPTION(4);
		TAKE_EXCEPTION(5);
		TAKE_EXCEPTION(6);
		TAKE_EXCEPTION(7);
#undef TAKE_EXCEPTION

		/* At most 7 + 40 bits, so never a whole word. */
		acc |= extra << acc_bits;
		acc_bits += 5 * count_high_bits(big);
		store64(exceptions + exceptions_size, acc);
		full_bytes = acc_bits >> 3;
		exceptions_size += full_bytes;
		acc >>= full_bytes * 8;
		acc_bits &= 7;

		for (j = 0; j < 3 && o + j < normal_size; j++) {
			normal[o + j] = (uint8_t) (packed >> (8 * j));
		}
	}
	if (acc_bits > 0) {
		exceptions[exceptions_size++] = (uint8_t) acc;
	}

	size = FPCODEC_HEADER_SIZE + normal_size + exceptions_size;
	if (out_len < size) {
		return FPCODEC_BUFFER_TOO_SMALL;
	}
	out[0] = version;
	out[1] = (uint8_t) (num_hashes >> 16);
	out[2] = (uint8_t) (num_hashes >> 8);
	out[3] = (uint8_t) num_hashes;
	memcpy(out + FPCODEC_HEADER_SIZE + normal_size, exceptions, exceptions_size);
	*encoded_size = size;
	return FPCODEC_OK;
}

fpcodec_status fpcodec_decode_header(const uint8_t *data, size_t data_len,
                                     uint8_t *version, size_t *num_hashes)
{
	size_t n;
	if (data_len < FPCODEC_HEADER_SIZE) {
		return FPCODEC_INVALID;
	}
	n = (size_t) data[1] << 16 | (size_t) data[2] << 8 | data[3];
	/* Each hash takes at least a 3-bit terminator. */
	if ((n * 3 + 7) / 8 > data_len - FPCODEC_HEADER_SIZE) {
		return FPCODEC_INVALID;
	}
	*version = data[0];
	*num_hashes = n;
	return FPCODEC_OK;
}

static size_t decode_values_size(size_t body_len)
{
	/* Every group of the body unpacked, the last one partial. */
	return (body_len + 2) / 3 * 8;
}

/* For the 16-bit reads that run past the end of the exception stream. */
#define EXCEPTIONS_PADDING 8

static size_t decode_exceptions_size(size_t body_len)
{
	return body_len + EXCEPTIONS_PADDING;
}

/*
 * The input past the header, up to the most a valid fingerprint of
 * `num_hashes` hashes can take. Anything after that is either trailing data
 * or part of an invalid fingerprint, which is then just as invalid without
 * it, so decoding work and scratch depend on the hash count and not on how
 * much is appended to the input.
 */
static size_t decode_body_len(size_t data_len, size_t num_hashes)
{
	const size_t body_len = data_len > FPCODEC_HEADER_SIZE ? data_len - FPCODEC_HEADER_SIZE : 0;
	const size_t max_len = fpcodec_max_encoded_size(num_hashes) - FPCODEC_HEADER_SIZE;
	return body_len < max_len ? body_len : max_len;
}

size_t fpcodec_decode_scratch_size(size_t data_len, size_t num_hashes)
{
	size_t body_len;
	if (num_hashes > FPCODEC_MAX_HASHES) {
		return 0;
	}
	body_len = decode_body_len(data_len, num_hashes);
	return decode_values_size(body_len) + decode_exceptions_size(body_len);
}

/*
 * Adds to each 7 among the eight values in `x` the next exception from the
 * packed 5-bit stream, starting with exception `*e`. 7s past the end of the
 * normal stream count too, so the stream needs padding for them.
 */
static inline uint64_t merge_exceptions(uint64_t x, const uint8_t *exceptions, size_t *e)
{
	uint64_t m = zero_bytes(x ^ SEVENS);
	while (m != 0) {
		const uint64_t low = m & (0 - m);
		const unsigned byte = count_high_bits((low - 1) & HIGHS);
		const size_t bit = *e * 5;
		const unsigned value = ((exceptions[bit / 8] | (unsigned) exceptions[bit / 8 + 1] << 8) >> (bit % 8)) & 0x1F;
		x += (uint64_t) value << (8 * byte);
		*e += 1;
		m ^= low;
	}
	return x;
}

fpcodec_status fpcodec_decode(const uint8_t *data, size_t data_len,
                              uint32_t *out, size_t out_len,
                              uint8_t *scratch, size_t scratch_len)
{
	const uint8_t *body;
	uint8_t *values, *exceptions, version;
	size_t num_hashes, body_len, max_values, num_values, num_exceptions = 0;
	size_t normal_size, exceptions_size, i, o, e, h;
	uint32_t pos, hash, invalid;
	fpcodec_status status;

	status = fpcodec_decode_header(data, data_len, &version, &num_hashes);
	if (status != FPCODEC_OK) {
		return status;
	}
	if (num_hashes == 0) {
		return FPCODEC_OK;
	}
	if (out_len < num_hashes || scratch_len < fpcodec_decode_scratch_size(data_len, num_hashes)) {
		return FPCODEC_BUFFER_TOO_SMALL;
	}
	body = data + FPCODEC_HEADER_SIZE;
	body_len = decode_body_len(data_len, num_hashes);
	values = scratch;
	exceptions = scratch + decode_values_size(body_len);

	/*
	 * Unpack everything as 3-bit values. Past the end of the normal stream
	 * that is the exception stream read wrong. Only the rest of the last
	 * group of eight is looked at, and its values are dropped.
	 */
	for (i = 0, o = 0; body_len - i >= 3; i += 3, o += 8) {
		store64(values + o, spread3(body[i] | (uint32_t) body[i + 1] << 8 | (uint32_t) body[i + 2] << 16));
	}
	if (i < body_len) {
		uint32_t x = body[i];
		if (body_len - i > 1) {
			x |= (uint32_t) body[i + 1] << 8;
		}
		store64(values + o, spread3(x));
	}
	max_values = body_len * 8 / 3;

	/* The normal stream ends at the terminator of the last hash. */
	{
		size_t remaining = num_hashes;
		for (i = 0; max_values - i >= 8; i += 8) {
			const uint64_t x = load64(values + i);
			const size_t zeros = count_high_bits(zero_bytes(x));
			if (zeros >= remaining) {
				break;
			}
			remaining -= zeros;
			num_exceptions += count_high_bits(zero_bytes(x ^ SEVENS));
		}
		for (;; i++) {
			if (i >= max_values) {
				return FPCODEC_INVALID;
			}
			if (values[i] == 0) {
				if (--remaining == 0) {
					break;
				}
			} else if (values[i] == 7) {
				num_exceptions++;
			}
		}
		num_values = i + 1;
	}

	normal_size = (num_values * 3 + 7) / 8;
	exceptions_size = (num_exceptions * 5 + 7) / 8;
	if (body_len - normal_size < exceptions_size) {
		return FPCODEC_INVALID;
	}
	memcpy(exceptions, body + normal_size, exceptions_size);
	memset(exceptions + exceptions_size, 0, EXCEPTIONS_PADDING);

	/*
	 * A hash is its XOR with the previous hash, XORed with the previous
	 * hash, so it is every bit decoded so far XORed together, taken at the
	 * hash's terminator. A bit's position is the sum of the distances since
	 * the last terminator. Every value writes the hash out, and the output
	 * position moves on at terminators only.
	 */
	pos = 0;
	hash = 0;
	e = 0;
	h = 0;
	invalid = 0;
#define DECODE_VALUE(x, j) \
	do { \
		const uint32_t delta = (uint32_t) ((x) >> (8 * (j))) & 0xFF; \
		const uint32_t not_end = 0 - (uint32_t) (delta != 0); \
		pos += delta; \
		invalid |= (uint32_t) (pos > 32) & not_end; \
		hash ^= ((uint32_t) 1 << ((pos - 1) & 31)) & not_end; \
		out[h] = hash; \
		h += 1 - (not_end & 1); \
		pos &= not_end; \
	} while (0)
	for (i = 0; num_values - i >= 8; i += 8) {
		const uint64_t x = merge_exceptions(load64(values + i), exceptions, &e);
		DECODE_VALUE(x, 0);
		DECODE_VALUE(x, 1);
		DECODE_VALUE(x, 2);
		DECODE_VALUE(x, 3);
		DECODE_VALUE(x, 4);
		DECODE_VALUE(x, 5);
		DECODE_VALUE(x, 6);
		DECODE_VALUE(x, 7);
		if (invalid) {
			return FPCODEC_INVALID;
		}
	}
	if (i < num_values) {
		const uint64_t x = merge_exceptions(load64(values + i), exceptions, &e);
		size_t j;
		for (j = 0; j < num_values - i; j++) {
			DECODE_VALUE(x, j);
		}
	}
#undef DECODE_VALUE
	return invalid ? FPCODEC_INVALID : FPCODEC_OK;
}
