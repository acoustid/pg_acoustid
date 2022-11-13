/* acoustid_fingerprint_encoding.c */

#include "acoustid.h"

#include "encode_fingerprint.h"
#include "base64.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_bytea);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_bytea);

Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS) {
    FingerprintType *input = PG_GETARG_FINGERPRINT_P(0);
    bytea *result;
    uint8_t *encoded_data;
    size_t encoded_size;

    encoded_data = (uint8_t *) VARDATA(input);
    encoded_size = VARSIZE(input) - VARHDRSZ;

    result = (bytea *) palloc(encoded_size + VARHDRSZ);
    memcpy(VARDATA(result), encoded_data, encoded_size);
    SET_VARSIZE(result, encoded_size + VARHDRSZ);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_BYTEA_P(result);
}

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS) {
    FingerprintType *input = PG_GETARG_FINGERPRINT_P(0);
    text *result;
    char *result_data_end;
    uint8_t *encoded_data;
    size_t encoded_size, result_data_size;

    encoded_data = (uint8_t *) VARDATA(input);
    encoded_size = VARSIZE(input) - VARHDRSZ;

    result_data_size = GetBase64EncodedSize(encoded_size);
    result = (text *) palloc(result_data_size);
    result_data_end = Base64Encode(encoded_data, encoded_data + encoded_size, VARDATA(result), 0);
    result_data_size = result_data_end - VARDATA(result);
    SET_VARSIZE(result, result_data_size + VARHDRSZ);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_TEXT_P(result);
}

Datum acoustid_fingerprint_decode_from_bytea(PG_FUNCTION_ARGS) {
    bytea *input = PG_GETARG_BYTEA_PP(0);
    FingerprintType *result;
    FingerprintData *tmp_fp;
    uint8_t *encoded_data;
    size_t encoded_size;

    encoded_data = (uint8_t *) VARDATA_ANY(input);
    encoded_size = VARSIZE_ANY_EXHDR(input);

    // Just for validation
    tmp_fp = decode_fingerprint(encoded_data, encoded_size);
    pfree(tmp_fp);

    result = (FingerprintType *) palloc(encoded_size + VARHDRSZ);
    memcpy(VARDATA(result), encoded_data, encoded_size);
    SET_VARSIZE(result, encoded_size + VARHDRSZ);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_FINGERPRINT_P(result);
}

Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS) {
    text *input = PG_GETARG_TEXT_PP(0);
    FingerprintType *result;
    FingerprintData *tmp_fp;
    uint8_t *encoded_data, *encoded_data_end;
    char *base64_encoded_data;
    size_t encoded_size, base64_encoded_size;
    text *input_text;

    base64_encoded_data = (char *) VARDATA_ANY(input);
    base64_encoded_size = VARSIZE_ANY_EXHDR(input);

    encoded_size = GetBase64DecodedSize(base64_encoded_size);
    result = (FingerprintType *) palloc(encoded_size + VARHDRSZ);
    encoded_data = (uint8_t *) VARDATA(result);
    encoded_data_end = Base64Decode(base64_encoded_data, base64_encoded_data + base64_encoded_size, encoded_data);
    encoded_size = encoded_data_end - encoded_data;
    SET_VARSIZE(result, encoded_size + VARHDRSZ);

    tmp_fp = decode_fingerprint(encoded_data, encoded_size);
    pfree(tmp_fp);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_FINGERPRINT_P(result);
}
