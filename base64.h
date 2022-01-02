// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_UTILS_BASE64_H_
#define CHROMAPRINT_UTILS_BASE64_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/*
 * URL-safe base64 encoding. This is similar to standard base64, but
 * characters '+' and '/' are replaced with '-' and '_'. Also, the padding
 * character '=' is omitted. The strings can be used in URLs without
 * any encoding.
 */

#define get_base64_encoded_size(size) (((size)*4 + 2) / 3)
#define get_base64_decoded_size(size) ((size)*3 / 4)

int base64_validate(const char *src, size_t src_size);
size_t base64_decode(const char *src, size_t src_size, uint8_t *dest, size_t dest_size);
size_t base64_encode(const uint8_t *src, size_t src_size, char *dest, size_t dest_size, int terminate);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CHROMAPRINT_UTILS_BASE64_H_ */
