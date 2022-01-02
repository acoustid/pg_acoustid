#include "base64.h"
#include "common.h"

static const char kBase64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const char kBase64CharsReversed[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 62, -1, -1, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,

    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
};

int base64_validate(const char *src, size_t src_size) {
    if (src_size % 4 == 1) {
        // We can't have one character left, since we couldn't retrieve a full byte from it.
        return -1;
    }
    for (size_t i = 0; i < src_size; ++i) {
        const uint8_t s = src[i];
        if (s > 127) {
            return -1;
        }
        if (kBase64CharsReversed[s] == -1) {
            return -1;
        }
    }
    return 0;
}

size_t base64_encode(const uint8_t *src, size_t src_size, char *dest, size_t dest_size, int terminate) {
    char *initial_dest = dest;
    while (src_size >= 3) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        CU_ASSERT(dest_size >= 4);
        dest[0] = kBase64Chars[(s0 >> 2) & 63];
        dest[1] = kBase64Chars[((s0 << 4) | (s1 >> 4)) & 63];
        dest[2] = kBase64Chars[((s1 << 2) | (s2 >> 6)) & 63];
        dest[3] = kBase64Chars[s2 & 63];
        src_size -= 3;
        src += 3;
        dest_size -= 4;
        dest += 4;
    }
    if (src_size == 2) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        CU_ASSERT(dest_size >= 3);
        dest[0] = kBase64Chars[(s0 >> 2) & 63];
        dest[1] = kBase64Chars[((s0 << 4) | (s1 >> 4)) & 63];
        dest[2] = kBase64Chars[(s1 << 2) & 63];
        dest += 3;
    } else if (src_size == 1) {
        const uint8_t s0 = src[0];
        CU_ASSERT(dest_size >= 2);
        dest[0] = kBase64Chars[(s0 >> 2) & 63];
        dest[1] = kBase64Chars[(s0 << 4) & 63];
        dest += 2;
    }
    if (terminate) {
        CU_ASSERT(dest_size >= 1);
        dest[0] = '\0';
    }
    return dest - initial_dest;
}

size_t base64_decode(const char *src, size_t src_size, uint8_t *dest, size_t dest_size) {
    uint8_t *initial_dest = dest;
    while (src_size >= 4) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t b0 = kBase64CharsReversed[s0];
        const uint8_t b1 = kBase64CharsReversed[s1];
        const uint8_t b2 = kBase64CharsReversed[s2];
        const uint8_t b3 = kBase64CharsReversed[s3];
        CU_ASSERT(dest_size >= 3);
        dest[0] = (b0 << 2) | (b1 >> 4);
        dest[1] = ((b1 << 4) & 255) | (b2 >> 2);
        dest[2] = ((b2 << 6) & 255) | b3;
        src_size -= 4;
        src += 4;
        dest_size -= 3;
        dest += 3;
    }
    if (src_size == 3) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t b0 = kBase64CharsReversed[s0];
        const uint8_t b1 = kBase64CharsReversed[s1];
        const uint8_t b2 = kBase64CharsReversed[s2];
        CU_ASSERT(dest_size >= 2);
        dest[0] = (b0 << 2) | (b1 >> 4);
        dest[1] = ((b1 << 4) & 255) | (b2 >> 2);
        dest += 2;
    } else if (src_size == 2) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t b0 = kBase64CharsReversed[s0];
        const uint8_t b1 = kBase64CharsReversed[s1];
        CU_ASSERT(dest_size >= 1);
        dest[0] = (b0 << 2) | (b1 >> 4);
        dest += 1;
    }
    return dest - initial_dest;
}
