// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

// This file was automatically generate using gen_bit_reader.py, do not edit.

#ifndef CHROMAPRINT_UTILS_UNPACK_INT3_ARRAY_H_
#define CHROMAPRINT_UTILS_UNPACK_INT3_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

#define GetUnpackedInt3ArraySize(size) ((size) * 8 / 3)

unsigned char *UnpackInt3Array(const unsigned char *first, const unsigned char *last, unsigned char *dest);

#endif
