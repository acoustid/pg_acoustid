// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

// This file was automatically generate using gen_bit_writer.py, do not edit.

#ifndef CHROMAPRINT_UTILS_PACK_INT3_ARRAY_H_
#define CHROMAPRINT_UTILS_PACK_INT3_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

inline size_t GetPackedInt3ArraySize(size_t size) { return (size * 3 + 7) / 8; }

unsigned char *PackInt3Array(const unsigned char *first, const unsigned char *last, unsigned char *dest);

#endif
