/* fingerprint_codec.h */

/*
 * Chromaprint's compressed fingerprint format, the one clients submit
 * (base64-encoded) and chromaprint_encode_fingerprint() produces.
 *
 * A header of the algorithm version (one byte) and the number of hashes (24
 * bits, big endian), then each hash XORed with the previous one, written as
 * the distances between its set bits: 3-bit values, where 0 ends a hash and
 * 7 means 7 plus the next 5-bit value from the exception stream. The 3-bit
 * stream comes first and the 5-bit stream right after it, both packed
 * lowest bit first.
 *
 * Decoding is stricter than Chromaprint 1.6.1 and earlier: a hash with a
 * set bit past bit 32 is invalid. So is a hash count the input is too short
 * to hold, which is checked with the header already, before anything is
 * allocated for the hashes. Bytes after the end of the fingerprint are
 * ignored, and a hash count of 0 is an empty fingerprint whatever follows
 * the header.
 *
 * This file depends on nothing but the C standard library, so that it can be
 * shared with other projects. It allocates no memory: the caller passes the
 * output and scratch buffers, sized with the functions below.
 */

#ifndef ACOUSTID_FINGERPRINT_CODEC_H_
#define ACOUSTID_FINGERPRINT_CODEC_H_

#include <stddef.h>
#include <stdint.h>

#define FPCODEC_HEADER_SIZE 4
#define FPCODEC_MAX_HASHES 0xFFFFFF

typedef enum {
	FPCODEC_OK = 0,
	/* The input is not a valid fingerprint, or has too many hashes. */
	FPCODEC_INVALID = -1,
	/* The output or scratch buffer is smaller than it has to be. */
	FPCODEC_BUFFER_TOO_SMALL = -2,
} fpcodec_status;

/*
 * Buffer sizes. Each is 0 for more than FPCODEC_MAX_HASHES hashes, which
 * the codec rejects before it touches any buffer.
 */

/* Output size `fpcodec_encode` never exceeds for `num_hashes` hashes. */
size_t fpcodec_max_encoded_size(size_t num_hashes);

/* Scratch size `fpcodec_encode` needs for `num_hashes` hashes. */
size_t fpcodec_encode_scratch_size(size_t num_hashes);

/*
 * Scratch size `fpcodec_decode` needs for `data_len` bytes of input with
 * the number of hashes `fpcodec_decode_header` returns. It does not grow
 * with data after the end of a fingerprint.
 */
size_t fpcodec_decode_scratch_size(size_t data_len, size_t num_hashes);

/*
 * Encodes `num_hashes` hashes into `out`, and stores the encoded size in
 * `*encoded_size`.
 */
fpcodec_status fpcodec_encode(uint8_t version, const uint32_t *hashes, size_t num_hashes,
                              uint8_t *out, size_t out_len, size_t *encoded_size,
                              uint8_t *scratch, size_t scratch_len);

/*
 * Reads the version and the number of hashes from the header, and checks
 * that the input is long enough to hold that many.
 */
fpcodec_status fpcodec_decode_header(const uint8_t *data, size_t data_len,
                                     uint8_t *version, size_t *num_hashes);

/*
 * Decodes the hashes into `out`, which has room for `out_len` hashes. The
 * number of hashes is the one `fpcodec_decode_header` returns.
 */
fpcodec_status fpcodec_decode(const uint8_t *data, size_t data_len,
                              uint32_t *out, size_t out_len,
                              uint8_t *scratch, size_t scratch_len);

#endif
