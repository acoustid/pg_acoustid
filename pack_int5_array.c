#include "pack_int5_array.h"

unsigned char *PackInt5Array(const unsigned char *first, const unsigned char *last, unsigned char *dest) {
    ptrdiff_t size = last - first;
    const unsigned char *src = first;
    while (size >= 8) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        const unsigned char s4 = *src++;
        const unsigned char s5 = *src++;
        const unsigned char s6 = *src++;
        const unsigned char s7 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        *dest++ = (unsigned char)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        *dest++ = (unsigned char)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1) | ((s6 & 0x03) << 6);
        *dest++ = (unsigned char)((s6 & 0x1c) >> 2) | ((s7 & 0x1f) << 3);
        size -= 8;
    }
    if (size == 7) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        const unsigned char s4 = *src++;
        const unsigned char s5 = *src++;
        const unsigned char s6 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        *dest++ = (unsigned char)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        *dest++ = (unsigned char)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1) | ((s6 & 0x03) << 6);
        *dest++ = (unsigned char)((s6 & 0x1c) >> 2);
    } else if (size == 6) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        const unsigned char s4 = *src++;
        const unsigned char s5 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        *dest++ = (unsigned char)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        *dest++ = (unsigned char)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1);
    } else if (size == 5) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        const unsigned char s4 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        *dest++ = (unsigned char)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        *dest++ = (unsigned char)((s4 & 0x10) >> 4);
    } else if (size == 4) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        const unsigned char s3 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        *dest++ = (unsigned char)((s3 & 0x1e) >> 1);
    } else if (size == 3) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2);
    } else if (size == 2) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        *dest++ = (unsigned char)((s1 & 0x18) >> 3);
    } else if (size == 1) {
        const unsigned char s0 = *src++;
        *dest++ = (unsigned char)((s0 & 0x1f));
    }
    return dest;
}
