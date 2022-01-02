#include "pg_acoustid.h"
#include "utils/base64.h"

Datum
acoustid_fingerprint_in(PG_FUNCTION_ARGS)
{
    const char *encoded = PG_GETARG_CSTRING(0);
    size_t encoded_size, decoded_size;
    char *decoded;
    Fingerprint *result;

    encoded_size = strlen(encoded);
    if (base64_validate(encoded, encoded_size) != 0) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("invalid base64 string")));
    }

    decoded_size = get_base64_decoded_size(encoded_size);
    result = palloc(decoded_size + VARHDRSZ);
    SET_VARSIZE(result, decoded_size + VARHDRSZ);
    decoded = VARDATA(result);
    base64_decode(encoded, encoded_size, decoded, decoded_size);

    PG_RETURN_FINGERPRINT_P(result);
}
