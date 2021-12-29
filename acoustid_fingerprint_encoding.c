/* acoustid_fingerprint_encoding.c */

#include "acoustid_fingerprint_encoding.h"

#include "encode_fingerprint.h"
#include "base64.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_encode_to_bytea);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_text);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_decode_from_bytea);

Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS) {
    Fingerprint *input = PG_GETARG_FINGERPRINT_P(0);
    bytea *result;
    uint8_t *encoded;
    size_t encoded_len;

    encode_fingerprint(input, &encoded, &encoded_len, VARHDRSZ);

    result = (bytea *)encoded;
    SET_VARSIZE(result, VARHDRSZ + encoded_len);

    PG_FREE_IF_COPY(input, 0);

    PG_RETURN_BYTEA_P(result);
}

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS) {
    Fingerprint *input = PG_GETARG_FINGERPRINT_P(0);
    text *result;
    char *result_data_end;
    uint8_t *encoded;
    size_t encoded_len, result_len;

    encode_fingerprint(input, &encoded, &encoded_len, 0);

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
    uint8_t *bytes;
    size_t bytes_len;
    Fingerprint *fp;

    input_bytea = PG_GETARG_BYTEA_PP(0);

    bytes = (uint8_t *)VARDATA_ANY(input_bytea);
    bytes_len = VARSIZE_ANY_EXHDR(input_bytea);

    fp = decode_fingerprint(bytes, bytes_len, NULL);

    PG_FREE_IF_COPY(input_bytea, 0);

    PG_RETURN_FINGERPRINT_P(fp);
}

Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS) {
    text *input_text;
    const char *str;
    uint8_t *tmp_bytes, *tmp_bytes_end;
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
