// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_UTILS_PACKED_UINT5_ARRAY_H_
#define CHROMAPRINT_UTILS_PACKED_UINT5_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define get_packed_uint5_array_size(size) (((size)*5 + 7) / 8)
#define get_unpacked_uint5_array_size(size) ((size)*8 / 5)

size_t pack_uint5_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size);
size_t unpack_uint5_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
