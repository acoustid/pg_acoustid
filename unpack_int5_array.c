#include "unpack_int5_array.h"

unsigned char *UnpackInt5Array(const unsigned char *first, const unsigned char *last, unsigned char *dest) {
    ptrdiff_t size = last - first;
    const unsigned char *src = first;
    while (size >= 5) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        const unsigned char s4 = *src++;
        *dest++ = (s0 & 0x1f);
        *dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        *dest++ = ((s1 & 0x7c) >> 2);
        *dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
        *dest++ = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
        *dest++ = ((s3 & 0x3e) >> 1);
        *dest++ = ((s3 & 0xc0) >> 6) | ((s4 & 0x07) << 2);
        *dest++ = ((s4 & 0xf8) >> 3);
        size -= 5;
    }
    if (size == 4) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        *dest++ = (s0 & 0x1f);
        *dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        *dest++ = ((s1 & 0x7c) >> 2);
        *dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
        *dest++ = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
        *dest++ = ((s3 & 0x3e) >> 1);
    } else if (size == 3) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        *dest++ = (s0 & 0x1f);
        *dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        *dest++ = ((s1 & 0x7c) >> 2);
        *dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
    } else if (size == 2) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        *dest++ = (s0 & 0x1f);
        *dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        *dest++ = ((s1 & 0x7c) >> 2);
    } else if (size == 1) {
        const unsigned char s0 = *src++;
        *dest++ = (s0 & 0x1f);
    }
    return dest;
}
