#ifndef CHROMAPRINT_UTILS_FINGERPRINT_ENCODING_H_
#define CHROMAPRINT_UTILS_FINGERPRINT_ENCODING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "common.h"

/*struct FingerprintCompressor {
    size_t num_terms;
    struct UInt8Vector normal_bits;
    struct UInt8Vector exceptional_bits;
};

void fingerprint_compressor_init(struct FingerprintCompressor *compressor);
void fingerprint_compressor_free(struct FingerprintCompressor *compressor);
*/

int compressed_fingerprint_get_version(const uint8_t *input, size_t input_size);
size_t compressed_fingerprint_get_num_terms(const uint8_t *input, size_t input_size);
int compressed_fingerprint_get_terms(const uint8_t *input, size_t input_size, uint32_t *output, size_t output_size);

// extern ssize_t get_encoded_fingerprint_size(const uint32_t *input, size_t input_size);

// extern ssize_t encode_fingerprint(const uint32_t *input, size_t input_size, uint8_t* output, size_t output_size);
// extern FingerprintData* decode_fingerprint(const uint8_t* input, size_t input_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
