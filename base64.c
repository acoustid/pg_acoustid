#include "base64.h"

static const char kBase64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const char kBase64CharsReversed[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 62, -1, -1, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,

    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,

    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,

    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

char *Base64Encode(const unsigned char *first, const unsigned char *last, char *dest, int terminate) {
    const unsigned char *src = first;
    ptrdiff_t size = last - first;
    while (size >= 3) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        const unsigned char s2 = *src++;
        *dest++ = kBase64Chars[(s0 >> 2) & 63];
        *dest++ = kBase64Chars[((s0 << 4) | (s1 >> 4)) & 63];
        *dest++ = kBase64Chars[((s1 << 2) | (s2 >> 6)) & 63];
        *dest++ = kBase64Chars[s2 & 63];
        size -= 3;
    }
    if (size == 2) {
        const unsigned char s0 = *src++;
        const unsigned char s1 = *src++;
        *dest++ = kBase64Chars[(s0 >> 2) & 63];
        *dest++ = kBase64Chars[((s0 << 4) | (s1 >> 4)) & 63];
        *dest++ = kBase64Chars[(s1 << 2) & 63];
    } else if (size == 1) {
        const unsigned char s0 = *src++;
        *dest++ = kBase64Chars[(s0 >> 2) & 63];
        *dest++ = kBase64Chars[(s0 << 4) & 63];
    }
    if (terminate) {
        *dest++ = '\0';
    }
    return dest;
}

unsigned char *Base64Decode(const char *first, const char *last, unsigned char *dest) {
    const char *src = first;
    ptrdiff_t size = last - first;
    while (size >= 4) {
        const unsigned char b0 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b1 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b2 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b3 = kBase64CharsReversed[*src++ & 255];
        *dest++ = (b0 << 2) | (b1 >> 4);
        *dest++ = ((b1 << 4) & 255) | (b2 >> 2);
        *dest++ = ((b2 << 6) & 255) | b3;
        size -= 4;
    }
    if (size == 3) {
        const unsigned char b0 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b1 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b2 = kBase64CharsReversed[*src++ & 255];
        *dest++ = (b0 << 2) | (b1 >> 4);
        *dest++ = ((b1 << 4) & 255) | (b2 >> 2);
    } else if (size == 2) {
        const unsigned char b0 = kBase64CharsReversed[*src++ & 255];
        const unsigned char b1 = kBase64CharsReversed[*src++ & 255];
        *dest++ = (b0 << 2) | (b1 >> 4);
    }
    return dest;
}
