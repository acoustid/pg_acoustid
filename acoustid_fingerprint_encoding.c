/* acoustid_fingerprint_encoding.c */

#include "acoustid_fingerprint_encoding.h"

#include "base64.h"
#include "pack_int3_array.h"
#include "pack_int5_array.h"
#include "unpack_int3_array.h"
#include "unpack_int5_array.h"

#include "utils/builtins.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_bytea);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_bytea);

static const int MAX_NORMAL_BIT_VALUE = (1 << 3) - 1;

void encode_fingerprint(Fingerprint *fp, uint8_t **output, size_t *output_len) {
    uint8_t *ptr;
    int i, num_terms;

    StringInfoData normal_bits;
    StringInfoData exceptional_bits;

    initStringInfo(&normal_bits);
    initStringInfo(&exceptional_bits);

    num_terms = FINGERPRINT_NTERMS(fp);

    for (i = 0; i < num_terms; i++) {
        int bit = 1, last_bit = 0;
        uint32_t term = (i == 0) ? FINGERPRINT_TERM(fp, i) : (FINGERPRINT_TERM(fp, i) ^ FINGERPRINT_TERM(fp, i - 1));
        while (term) {
            if ((term & 1) != 0) {
                int value = bit - last_bit;
                if (value >= MAX_NORMAL_BIT_VALUE) {
                    appendStringInfoChar(&normal_bits, MAX_NORMAL_BIT_VALUE);
                    appendStringInfoChar(&exceptional_bits, value - MAX_NORMAL_BIT_VALUE);
                } else {
                    appendStringInfoChar(&normal_bits, value);
                }
                last_bit = bit;
            }
            term >>= 1;
            bit++;
        }
        appendStringInfoChar(&normal_bits, 0);
    }

    *output_len = 4 + GetPackedInt3ArraySize(normal_bits.len) + GetPackedInt5ArraySize(exceptional_bits.len);
    *output = palloc0(*output_len);

    ptr = *output;

    ptr[0] = 0;
    ptr[1] = (num_terms >> 16) & 255;
    ptr[2] = (num_terms >> 8) & 255;
    ptr[3] = (num_terms)&255;
    ptr += 4;

    ptr = PackInt3Array((const uint8_t *)normal_bits.data, (const uint8_t *)normal_bits.data + normal_bits.len, ptr);
    ptr = PackInt5Array((const uint8_t *)exceptional_bits.data,
                        (const uint8_t *)exceptional_bits.data + exceptional_bits.len, ptr);

    *output_len = ptr - *output;
}

Fingerprint *decode_fingerprint(const unsigned char *input, int input_len, int *version) {
    Fingerprint *fp;
    int i, j, num_terms, num_bits, num_exceptional_bits, found_terms, bit, last_bit, cursor = 0;
    uint32_t term, last_term;
    const unsigned char *header;
    unsigned char *bits, *exceptional_bits;

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
    num_terms = ((uint32)header[1] << 16) | ((uint32)header[2] << 8) | header[3];

    // Estimate the number of bits stored in the packed fingerprint
    num_bits = GetUnpackedInt3ArraySize(input_len - cursor);
    bits = palloc(num_bits * sizeof(unsigned char));

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
            palloc(GetUnpackedInt5ArraySize(GetPackedInt5ArraySize(num_exceptional_bits)) * sizeof(unsigned char));
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

Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS) {
    Fingerprint *input = PG_GETARG_FINGERPRINT_P(0);
    bytea *result;
    uint8_t *encoded;
    size_t encoded_len;

    encode_fingerprint(input, &encoded, &encoded_len);

    result = (bytea *)palloc(VARHDRSZ + encoded_len);
    SET_VARSIZE(result, VARHDRSZ + encoded_len);
    memcpy(VARDATA(result), encoded, encoded_len);

    pfree(encoded);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_BYTEA_P(result);
}

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS) {
    Fingerprint *input = PG_GETARG_FINGERPRINT_P(0);
    text *result;
    char *result_data_end;
    uint8_t *encoded;
    size_t encoded_len, result_len;

    encode_fingerprint(input, &encoded, &encoded_len);

    result_len = GetBase64EncodedSize(encoded_len);
    result = (text *)palloc(VARHDRSZ + result_len);

    result_data_end = Base64Encode(encoded, encoded + encoded_len, VARDATA(result), 1);
    result_len = result_data_end - VARDATA(result);
    SET_VARSIZE(result, VARHDRSZ + result_len);

    pfree(encoded);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_TEXT_P(result);
}

Datum acoustid_fingerprint_decode_from_bytea(PG_FUNCTION_ARGS) {
    bytea *input_bytea;
    unsigned char *bytes;
    size_t bytes_len;
    Fingerprint *fp;

    input_bytea = PG_GETARG_BYTEA_PP(0);

    bytes = (unsigned char *)VARDATA_ANY(input_bytea);
    bytes_len = VARSIZE_ANY_EXHDR(input_bytea);

    fp = decode_fingerprint(bytes, bytes_len, NULL);

    PG_FREE_IF_COPY(input_bytea, 0);

    PG_RETURN_FINGERPRINT_P(fp);
}

Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS) {
    text *input_text;
    const char *str;
    unsigned char *tmp_bytes, *tmp_bytes_end;
    size_t str_len, tmp_bytes_len;
    Fingerprint *fp;

    input_text = PG_GETARG_TEXT_PP(0);

    str = VARDATA_ANY(input_text);
    str_len = VARSIZE_ANY_EXHDR(input_text);

    tmp_bytes_len = GetBase64DecodedSize(str_len);
    tmp_bytes = palloc(tmp_bytes_len);

    tmp_bytes_end = Base64Decode(str, str + str_len, tmp_bytes);
    tmp_bytes_len = tmp_bytes_end - tmp_bytes;

    fp = decode_fingerprint(tmp_bytes, tmp_bytes_len, NULL);
    pfree(tmp_bytes);

    PG_FREE_IF_COPY(input_text, 0);

    PG_RETURN_FINGERPRINT_P(fp);
}
