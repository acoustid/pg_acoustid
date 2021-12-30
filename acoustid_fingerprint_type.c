/* acoustid_fingerprint_type.c */

#include "acoustid.h"

#include "encode_fingerprint.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_type_in);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_type_out);

Datum acoustid_fingerprint_type_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    FingerprintData *fp;
    FingerprintType *result;
    uint8_t *encoded_data;
    size_t encoded_size;

    fp = fingerprint_data_from_string(str);
    encode_fingerprint(fp, &encoded_data, &encoded_size, VARHDRSZ);

    result = (FingerprintType *) encoded_data;
    SET_VARSIZE(result, encoded_size + VARHDRSZ);

    PG_RETURN_FINGERPRINT_P(result);
}

Datum acoustid_fingerprint_type_out(PG_FUNCTION_ARGS) {
    FingerprintType *input = PG_GETARG_FINGERPRINT_P(0);
    FingerprintData *fp;
    char *result;
    uint8_t *encoded_data;
    size_t encoded_size;

    encoded_data = (uint8_t *) VARDATA(input);
    encoded_size = VARSIZE(input) - VARHDRSZ;

    fp = decode_fingerprint(encoded_data, encoded_size);
    result = fingerprint_data_to_string(fp);

    PG_RETURN_CSTRING(result);
}
