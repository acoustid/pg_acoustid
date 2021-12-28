// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

// This file was automatically generate using gen_bit_reader.py, do not edit.

#ifndef CHROMAPRINT_UTILS_UNPACK_INT3_ARRAY_H_
#define CHROMAPRINT_UTILS_UNPACK_INT3_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

inline size_t GetUnpackedInt3ArraySize(size_t size) { return size * 8 / 3; }

inline unsigned char *UnpackInt3Array(const unsigned char *first, const unsigned char *last, unsigned char *dest) {
    ptrdiff_t size = last - first;
    const unsigned char *src = first;
    while (size >= 3) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        *dest++ = (s0 & 0x07);
        *dest++ = ((s0 & 0x38) >> 3);
        *dest++ = ((s0 & 0xc0) >> 6) | ((s1 & 0x01) << 2);
        *dest++ = ((s1 & 0x0e) >> 1);
        *dest++ = ((s1 & 0x70) >> 4);
        *dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x03) << 1);
        *dest++ = ((s2 & 0x1c) >> 2);
        *dest++ = ((s2 & 0xe0) >> 5);
        size -= 3;
    }
    if (size == 2) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        *dest++ = (s0 & 0x07);
        *dest++ = ((s0 & 0x38) >> 3);
        *dest++ = ((s0 & 0xc0) >> 6) | ((s1 & 0x01) << 2);
        *dest++ = ((s1 & 0x0e) >> 1);
        *dest++ = ((s1 & 0x70) >> 4);
    } else if (size == 1) {
        const unsigned char s0 = *src++;
        *dest++ = (s0 & 0x07);
        *dest++ = ((s0 & 0x38) >> 3);
    }
    return dest;
}

#endif
