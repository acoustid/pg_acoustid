// Copyright (C) 2021  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_UTILS_PACKED_UINT3_ARRAY_H_
#define CHROMAPRINT_UTILS_PACKED_UINT3_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define get_packed_uint3_array_size(size) (((size)*3 + 7) / 8)
#define get_unpacked_uint3_array_size(size) ((size)*8 / 3)

size_t pack_uint3_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size);
size_t unpack_uint3_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
