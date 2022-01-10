#include "fingerprint_compression.h"
#include "packed_uint3_array.h"
#include "packed_uint5_array.h"

const uint8_t UINT3_MAX = 7;

int compressed_fingerprint_get_version(const uint8_t *input, size_t input_size) {
    if (input_size < 1) {
        return -1;
    }
    return input[0];
}

size_t compressed_fingerprint_get_num_terms(const uint8_t *input, size_t input_size) {
    if (input_size < 4) {
        return 0;
    }
    return input[3] | (input[2] << 8) | (input[1] << 16);
}

int compressed_fingerprint_get_terms(const uint8_t *input, size_t input_size, uint32_t *terms,
                                     size_t num_allocated_terms) {
    size_t i, j, cursor = 0, num_terms, num_found_terms = 0, num_bits, num_normal_bits = 0, num_exceptional_bits = 0;
    int bit, last_bit;
    uint32_t term, last_term;
    uint8_t *bits;

    if (input_size < 4) {
        CU_LOG_ERROR("Invalid ingerprint, needs at least 4 bytes of data");
        return -1;
    }

    CU_ASSERT(cursor <= input_size);

    num_terms = compressed_fingerprint_get_num_terms(input, input_size - cursor);
    if (num_terms > num_allocated_terms) {
        CU_LOG_ERROR("Not enough space allocated for terms, needs %zu, allocated %zu", num_terms, num_allocated_terms);
        return -1;
    }

    cursor += 4;
    CU_ASSERT(cursor <= input_size);

    num_bits = get_unpacked_uint3_array_size(input_size - cursor);
    bits = (uint8_t *)CU_ALLOC(num_bits);
    unpack_uint3_array(input + cursor, input_size - cursor, bits, num_bits);

    for (i = 0; i < num_bits; i++) {
        if (bits[i] == 0) {
            num_found_terms++;
            if (num_found_terms == num_terms) {
                num_normal_bits = i + 1;
                break;
            }
        } else {
            num_normal_bits++;
            if (bits[i] == UINT3_MAX) {
                num_exceptional_bits++;
            }
        }
    }

    if (num_found_terms != num_terms) {
        CU_LOG_ERROR("Invalid fingerprint, did not have enough data for normal bits");
        CU_FREE(bits);
        return -1;
    }

    cursor += get_packed_uint3_array_size(num_normal_bits);
    CU_ASSERT(cursor <= input_size);

    CU_LOG_DEBUG("Found %zu normal bits, %zu exceptional bits", num_normal_bits, num_exceptional_bits);

    if (get_packed_uint5_array_size(num_exceptional_bits) > input_size - cursor) {
        CU_LOG_ERROR("Invalid fingerprint, did not have enough data for exceptional bits");
        CU_FREE(bits);
        return -1;
    }

    if (get_packed_uint5_array_size(num_exceptional_bits) < input_size - cursor) {
        CU_LOG_ERROR("Invalid fingerprint, found trailing data");
        CU_FREE(bits);
        return -1;
    }

    CU_ASSERT(num_bits >= num_normal_bits + num_exceptional_bits);

    unpack_uint5_array(input + cursor, input_size - cursor, bits + num_normal_bits, num_bits - num_normal_bits);

    for (i = 0, j = num_normal_bits; i < num_normal_bits && j < num_bits; i++) {
        if (bits[i] == UINT3_MAX) {
            bits[i] += bits[j++];
        }
    }

    last_term = 0;
    last_bit = 0;
    term = 0;
    for (i = 0, j = 0; i < num_bits && j < num_terms; i++) {
        bit = bits[i];
        if (bit == 0) {
            last_term ^= term;
            terms[j] = last_term;
            last_bit = 0;
            term = 0;
            j++;
        } else {
            bit += last_bit;
            last_bit = bit;
            term |= 1 << (bit - 1);
        }
    }

    CU_ASSERT(j == num_terms);

    CU_FREE(bits);

    return 0;
}
