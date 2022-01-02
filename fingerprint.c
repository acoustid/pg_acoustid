#include "pg_acoustid.h"
#include "utils/base64.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_in);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_out);

PG_FUNCTION_INFO_V1(acoustid_fingerprint_cast_to_bytea);

Datum
acoustid_fingerprint_in(PG_FUNCTION_ARGS)
{
    const char *encoded = PG_GETARG_CSTRING(0);
    size_t encoded_size, decoded_size;
    unsigned char *decoded;
    FingerprintType *fingerprint;

    encoded_size = strlen(encoded);
    if (base64_validate(encoded, encoded_size) != 0) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("invalid base64 string")));
    }

    decoded_size = get_base64_decoded_size(encoded_size);
    fingerprint = palloc(decoded_size + VARHDRSZ);
    SET_VARSIZE(fingerprint, decoded_size + VARHDRSZ);
    decoded = (unsigned char *) VARDATA(fingerprint);

    base64_decode(encoded, encoded_size, decoded, decoded_size);

    PG_RETURN_FINGERPRINT_P(fingerprint);
}

Datum
acoustid_fingerprint_out(PG_FUNCTION_ARGS)
{
    FingerprintType *fingerprint = PG_GETARG_FINGERPRINT_PP(0);
    size_t encoded_size, decoded_size;
    char *encoded;
    unsigned char *decoded;

    decoded_size = VARSIZE(fingerprint) - VARHDRSZ;
    decoded = (unsigned char *) VARDATA(fingerprint);

    encoded_size = get_base64_encoded_size(decoded_size);
    encoded = palloc(encoded_size + 1);  /* +1 for null terminator */

    base64_encode(decoded, decoded_size, encoded, encoded_size, 1);

    PG_FREE_IF_COPY(fingerprint, 0);

    PG_RETURN_CSTRING(encoded);
}

Datum
acoustid_fingerprint_cast_to_bytea(PG_FUNCTION_ARGS)
{
    FingerprintType *fingerprint = PG_GETARG_FINGERPRINT_PP(0);
    bytea *result;

    result = (bytea *) palloc(VARSIZE(fingerprint));
    memcpy(result, fingerprint, VARSIZE(fingerprint));

    PG_FREE_IF_COPY(fingerprint, 0);

    PG_RETURN_BYTEA_P(result);
}
