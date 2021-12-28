/* acoustid_fingerprint_encoding.c */

#include "acoustid_fingerprint_encoding.h"

#include "pack_int3_array.h"
#include "pack_int5_array.h"
#include "unpack_int3_array.h"
#include "unpack_int5_array.h"

#include "urlsafe_base64.h"

#include "utils/builtins.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode);

static const int MAX_NORMAL_BIT_VALUE = (1 << 3) - 1;

Fingerprint *decode_fingerprint(const unsigned char *input, int input_len, int *version) {
    Fingerprint *fp;
    int i, j, num_terms, num_bits, num_exceptional_bits, found_terms, bit,
        last_bit, cursor = 0;
    uint32_t term, last_term;
    const unsigned char *header;
    unsigned char *bits, *exceptional_bits;

    elog(DEBUG5, "decode_fingerprint: input_len=%d", input_len);

    if (cursor + 4 > input_len) {
        elog(ERROR, "Invalid fingerprint (shorter than 4 bytes)");
        return NULL;
    }

    // Read the header
    header = input + cursor;
    cursor += 4;
    if (version) {
        *version = header[0];
    }
    num_terms = ((uint32) header[1] << 16) | ((uint32) header[2] << 8) | header[3];
    elog(DEBUG5, "decode_fingerprint: num_terms=%d", num_terms);

    // Estimate the number of bits stored in the packed fingerprint
    num_bits = GetUnpackedInt3ArraySize(input_len - cursor);
    bits = palloc(num_bits * sizeof(unsigned char));
    elog(DEBUG5, "decode_fingerprint: num_bits=%d (estimate)", num_bits);

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
    elog(DEBUG5, "decode_fingerprint: num_bits=%d, num_exceptional_bits=%d",
        num_bits, num_exceptional_bits);

    // Advance the cursor to the end of the normal bits
    cursor += GetPackedInt3ArraySize(num_bits);

    if (found_terms != num_terms) {
        elog(INFO, "num_terms=%d found_terms=%d", num_terms, found_terms);

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
        // Estimate the number of exceptional bits stored in the packed fingerprint
        exceptional_bits =
            palloc(GetUnpackedInt5ArraySize(
                       GetPackedInt5ArraySize(num_exceptional_bits)) *
                   sizeof(unsigned char));
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

    fp = create_empty_fingerprint(num_terms);

    // Unpack the bits into fingerprint terms
    last_term = 0;
    last_bit = 0;
    term = 0;
    for (i = 0, j = 0; i < num_bits && j < num_terms; i++) {
        bit = bits[i];
        if (bit == 0) {
            last_term ^= term;
            FINGERPRINT_TERM(fp, j) = last_term;
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

Datum acoustid_fingerprint_decode(PG_FUNCTION_ARGS) {
    text *input;
    const char *str;
    unsigned char *bytes;
    int str_len, bytes_len, ret;
    Fingerprint *fp;

    input = PG_GETARG_TEXT_P(0);
    str = text_to_cstring(input);
    str_len = strlen(str);

    bytes_len = urlsafe_b64_dec_len(str_len);
    bytes = palloc(bytes_len);

    ret = urlsafe_b64_decode(str, str_len, (char *) bytes, bytes_len);
    if (ret < 0) {
        pfree(bytes);
        elog(ERROR, "Invalid fingerprint (invalid base64)");
        PG_RETURN_NULL();
    }

    fp = decode_fingerprint(bytes, bytes_len, NULL);
    pfree(bytes);
    PG_RETURN_FINGERPRINT_P(fp);
}
