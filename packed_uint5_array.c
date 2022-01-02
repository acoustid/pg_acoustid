#include "packed_uint5_array.h"
#include "common.h"

size_t pack_uint5_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size) {
    uint8_t *initial_dest = dest;
    while (src_size >= 8) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t s4 = src[4];
        const uint8_t s5 = src[5];
        const uint8_t s6 = src[6];
        const uint8_t s7 = src[7];
        CU_ASSERT(dest_size >= 5);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        dest[2] = (uint8_t)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        dest[3] = (uint8_t)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1) | ((s6 & 0x03) << 6);
        dest[4] = (uint8_t)((s6 & 0x1c) >> 2) | ((s7 & 0x1f) << 3);
        src_size -= 8;
        src += 8;
        dest_size -= 5;
        dest += 5;
    }
    if (src_size == 7) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t s4 = src[4];
        const uint8_t s5 = src[5];
        const uint8_t s6 = src[6];
        CU_ASSERT(dest_size >= 5);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        dest[2] = (uint8_t)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        dest[3] = (uint8_t)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1) | ((s6 & 0x03) << 6);
        dest[4] = (uint8_t)((s6 & 0x1c) >> 2);
        dest += 5;
    } else if (src_size == 6) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t s4 = src[4];
        const uint8_t s5 = src[5];
        CU_ASSERT(dest_size >= 4);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        dest[2] = (uint8_t)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        dest[3] = (uint8_t)((s4 & 0x10) >> 4) | ((s5 & 0x1f) << 1);
        dest += 4;
    } else if (src_size == 5) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t s4 = src[4];
        CU_ASSERT(dest_size >= 4);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        dest[2] = (uint8_t)((s3 & 0x1e) >> 1) | ((s4 & 0x0f) << 4);
        dest[3] = (uint8_t)((s4 & 0x10) >> 4);
        dest += 4;
    } else if (src_size == 4) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        CU_ASSERT(dest_size >= 3);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2) | ((s3 & 0x01) << 7);
        dest[2] = (uint8_t)((s3 & 0x1e) >> 1);
        dest += 3;
    } else if (src_size == 3) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        CU_ASSERT(dest_size >= 2);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3) | ((s2 & 0x1f) << 2);
        dest += 2;
    } else if (src_size == 2) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        CU_ASSERT(dest_size >= 2);
        dest[0] = (uint8_t)((s0 & 0x1f)) | ((s1 & 0x07) << 5);
        dest[1] = (uint8_t)((s1 & 0x18) >> 3);
        dest += 2;
    } else if (src_size == 1) {
        const uint8_t s0 = src[0];
        CU_ASSERT(dest_size >= 1);
        dest[0] = (uint8_t)((s0 & 0x1f));
        dest += 1;
    }
    return dest - initial_dest;
}

size_t unpack_uint5_array(const uint8_t *src, size_t src_size, uint8_t *dest, size_t dest_size) {
    uint8_t *initial_dest = dest;
    while (src_size >= 5) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        const uint8_t s4 = src[4];
        CU_ASSERT(dest_size >= 8);
        dest[0] = (s0 & 0x1f);
        dest[1] = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        dest[2] = ((s1 & 0x7c) >> 2);
        dest[3] = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
        dest[4] = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
        dest[5] = ((s3 & 0x3e) >> 1);
        dest[6] = ((s3 & 0xc0) >> 6) | ((s4 & 0x07) << 2);
        dest[7] = ((s4 & 0xf8) >> 3);
        src_size -= 5;
        src += 5;
        dest_size -= 8;
        dest += 8;
    }
    if (src_size == 4) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        const uint8_t s3 = src[3];
        CU_ASSERT(dest_size >= 6);
        dest[0] = (s0 & 0x1f);
        dest[1] = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        dest[2] = ((s1 & 0x7c) >> 2);
        dest[3] = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
        dest[4] = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
        dest[5] = ((s3 & 0x3e) >> 1);
        dest += 6;
    } else if (src_size == 3) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        const uint8_t s2 = src[2];
        CU_ASSERT(dest_size >= 4);
        dest[0] = (s0 & 0x1f);
        dest[1] = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        dest[2] = ((s1 & 0x7c) >> 2);
        dest[3] = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
        dest += 4;
    } else if (src_size == 2) {
        const uint8_t s0 = src[0];
        const uint8_t s1 = src[1];
        CU_ASSERT(dest_size >= 3);
        dest[0] = (s0 & 0x1f);
        dest[1] = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
        dest[2] = ((s1 & 0x7c) >> 2);
        dest += 3;
    } else if (src_size == 1) {
        const uint8_t s0 = src[0];
        CU_ASSERT(dest_size >= 1);
        dest[0] = (s0 & 0x1f);
        dest += 1;
    }
    return dest - initial_dest;
}
