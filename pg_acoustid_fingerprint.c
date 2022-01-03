#include "pg_acoustid.h"
#include "utils/base64.h"
#include "utils/fingerprint_compression.h"

#include "catalog/pg_type.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_in);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_out);

PG_FUNCTION_INFO_V1(acoustid_fingerprint_extract_version);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_extract_terms);

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
acoustid_fingerprint_extract_version(PG_FUNCTION_ARGS)
{
    FingerprintType *fingerprint = PG_GETARG_FINGERPRINT_PP(0);
    unsigned char *data;
    size_t size;
    int version;

    size = VARSIZE(fingerprint) - VARHDRSZ;
    data = (unsigned char *) VARDATA(fingerprint);

    version = compressed_fingerprint_get_version(data, size);

    PG_FREE_IF_COPY(fingerprint, 0);

    PG_RETURN_INT32(version);
}

Datum
acoustid_fingerprint_extract_terms(PG_FUNCTION_ARGS)
{
    FingerprintType *fingerprint = PG_GETARG_FINGERPRINT_PP(0);
    ArrayType *terms_array;
    unsigned char *data;
    uint32_t *terms;
    size_t size, num_terms, terms_array_size;
    int ret;

    size = VARSIZE(fingerprint) - VARHDRSZ;
    data = (unsigned char *) VARDATA(fingerprint);

    num_terms = compressed_fingerprint_get_num_terms(data, size);

    terms_array_size = ARR_OVERHEAD_NONULLS(1) + sizeof(uint32_t) * num_terms;
    terms_array = (ArrayType *) palloc(terms_array_size);

    SET_VARSIZE(terms_array, terms_array_size);
    terms_array->ndim = 1;
    terms_array->dataoffset = 0;
    terms_array->elemtype = INT4OID;
    ARR_DIMS(terms_array)[0] = num_terms;
    ARR_LBOUND(terms_array)[0] = 1;

    terms = (uint32_t *) ARR_DATA_PTR(terms_array);

    ret = compressed_fingerprint_get_terms(data, size, terms, num_terms);
    if (ret != 0) {
        pfree(terms_array);
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("invalid fingerprint")));
    }

    PG_FREE_IF_COPY(fingerprint, 0);

    PG_RETURN_ARRAYTYPE_P(terms_array);
}
