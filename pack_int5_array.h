// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

// This file was automatically generate using gen_bit_writer.py, do not edit.

#ifndef CHROMAPRINT_UTILS_PACK_INT5_ARRAY_H_
#define CHROMAPRINT_UTILS_PACK_INT5_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

#define GetPackedInt5ArraySize(size) (((size) * 5 + 7) / 8)

unsigned char *PackInt5Array(const unsigned char *first, const unsigned char *last, unsigned char *dest);

#endif
