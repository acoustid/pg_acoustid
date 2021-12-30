#include "encode_fingerprint.h"

#include "pack_int3_array.h"
#include "pack_int5_array.h"
#include "unpack_int3_array.h"
#include "unpack_int5_array.h"
#include "uint8_vector.h"

static const int MAX_NORMAL_BIT_VALUE = (1 << 3) - 1;

void encode_fingerprint(FingerprintData *fp, uint8_t **output, size_t *output_len, size_t extra_header) {
    uint8_t *ptr;
    uint32_t last_term;
    int i, num_terms;

    UInt8Vector normal_bits;
    UInt8Vector exceptional_bits;

    num_terms = fp->size;

    uint8_vector_alloc(&normal_bits, num_terms);
    uint8_vector_alloc(&exceptional_bits, num_terms);

    last_term = 0;
    for (i = 0; i < num_terms; i++) {
        int bit = 1, last_bit = 0;
        uint32_t term = fp->terms[i];
        uint32_t term_bits = term ^ last_term;
        last_term = term;
        while (term_bits) {
            if ((term_bits & 1) != 0) {
                int value = bit - last_bit;
                if (value >= MAX_NORMAL_BIT_VALUE) {
                    uint8_vector_push_back(&normal_bits, MAX_NORMAL_BIT_VALUE);
                    uint8_vector_push_back(&exceptional_bits, value - MAX_NORMAL_BIT_VALUE);
                } else {
                    uint8_vector_push_back(&normal_bits, value);
                }
                last_bit = bit;
            }
            term_bits >>= 1;
            bit++;
        }
        uint8_vector_push_back(&normal_bits, 0);
    }

    *output_len = 4 + GetPackedInt3ArraySize(normal_bits.size) + GetPackedInt5ArraySize(exceptional_bits.size);
    *output = palloc0(*output_len + extra_header);

    ptr = *output + extra_header;

    ptr[0] = fp->version;
    ptr[1] = num_terms >> 16;
    ptr[2] = num_terms >> 8;
    ptr[3] = num_terms;
    ptr += 4;

    ptr = PackInt3Array(normal_bits.data, normal_bits.data + normal_bits.size, ptr);
    ptr = PackInt5Array(exceptional_bits.data, exceptional_bits.data + exceptional_bits.size, ptr);

    uint8_vector_free(&normal_bits);
    uint8_vector_free(&exceptional_bits);

    *output_len = ptr - (*output + extra_header);
}

FingerprintData *decode_fingerprint(const uint8_t *input, size_t input_len) {
    FingerprintData *fp;
    int i, j, version, num_terms, num_bits, num_exceptional_bits, found_terms, bit, last_bit, cursor = 0;
    uint32_t term, last_term;
    const uint8_t *header;
    uint8_t *bits, *exceptional_bits;

    if (cursor + 4 > input_len) {
        elog(ERROR, "Invalid fingerprint (shorter than 4 bytes)");
        return NULL;
    }

    // Read the header
    header = input + cursor;
    version = header[0];
    num_terms = ((uint32_t)header[1] << 16) | ((uint32_t)header[2] << 8) | header[3];
    cursor += 4;

    // Estimate the number of bits stored in the packed fingerprint
    num_bits = GetUnpackedInt3ArraySize(input_len - cursor);
    bits = palloc(num_bits * sizeof(uint8_t));

    // Unpack the normal bits
    UnpackInt3Array(input + cursor, input + input_len, bits);

    // Find the actual number of normal bits, count the number of exceptional
    // bits
    found_terms = 0;
    num_exceptional_bits = 0;
    for (i = 0; i < num_bits; i++) {
        if (bits[i] == 0) {
            found_terms++;
            if (found_terms == num_terms) {
                num_bits = i + 1;
                break;
            }
        } else if (bits[i] == MAX_NORMAL_BIT_VALUE) {
            num_exceptional_bits++;
        }
    }

    // Advance the cursor to the end of the normal bits
    cursor += GetPackedInt3ArraySize(num_bits);

    if (found_terms != num_terms) {
        pfree(bits);
        elog(ERROR, "Invalid fingerprint (too short, not enough input for "
                    "normal bits)");
        return NULL;
    }

    if (cursor + GetPackedInt5ArraySize(num_exceptional_bits) > input_len) {
        pfree(bits);
        elog(ERROR, "Invalid fingerprint (too short, not enough input for "
                    "exceptional bits)");
        return NULL;
    }

    if (num_exceptional_bits > 0) {
        // Estimate the number of exceptional bits stored in the packed
        // fingerprint
        exceptional_bits =
            palloc(GetUnpackedInt5ArraySize(GetPackedInt5ArraySize(num_exceptional_bits)) * sizeof(uint8_t));
        // Unpack the exceptional bits
        UnpackInt5Array(input + cursor, input + input_len, exceptional_bits);
        // Add the exceptional bits to the normal bits
        for (i = 0, j = 0; i < num_bits && j < num_exceptional_bits; i++) {
            if (bits[i] == MAX_NORMAL_BIT_VALUE) {
                bits[i] += exceptional_bits[j++];
            }
        }
        pfree(exceptional_bits);
    }

    fp = fingerprint_data_alloc(num_terms);
    fp->version = version;
    fp->size = num_terms;

    // Unpack the bits into fingerprint terms
    last_term = 0;
    last_bit = 0;
    term = 0;
    for (i = 0, j = 0; i < num_bits && j < num_terms; i++) {
        bit = bits[i];
        if (bit == 0) {
            last_term ^= term;
            fp->terms[j] = last_term;
            last_bit = 0;
            term = 0;
            j++;
        } else {
            bit += last_bit;
            last_bit = bit;
            term |= 1 << (bit - 1);
        }
    }

    pfree(bits);
    return fp;
}
