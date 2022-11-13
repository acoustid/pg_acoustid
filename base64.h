// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_BASE64_H_
#define CHROMAPRINT_BASE64_H_

#include <stddef.h>
#include <stdint.h>

inline size_t GetBase64EncodedSize(size_t size) { return (size * 4 + 2) / 3; }

inline size_t GetBase64DecodedSize(size_t size) { return size * 3 / 4; }

char *Base64Encode(const unsigned char *first, const unsigned char *last, char *dest, int terminate);
unsigned char *Base64Decode(const char *first, const char *last, unsigned char *dest);

#endif
