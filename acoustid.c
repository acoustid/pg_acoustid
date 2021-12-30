/* acoustid.c */

#include "acoustid.h"


#include "utils/array.h"
#include "catalog/pg_type.h"

PG_MODULE_MAGIC;

/*
PG_FUNCTION_INFO_V1(acoustid_fingerprint_from_int4array);

Datum acoustid_fingerprint_from_int4array(PG_FUNCTION_ARGS) {
    int version = PG_GETARG_INT32(0);
    ArrayType *input = PG_GETARG_ARRAYTYPE_P(1);

    size_t num_terms;
    uint32_t *terms;

    if (ARR_ELEMTYPE(input) != INT4OID) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("array must contain int4 values")));
    }

    if (ARR_NDIM(input) != 1) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("array must be one-dimensional")));
    }

    if (ARR_HASNULL(input)) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("array must not contain nulls")));
    }

    num_terms = ARR_DIMS(input)[0];
    terms = (uint32_t *)ARR_DATA_PTR(input);

    PG_RETURN_FINGERPRINT_P(create_fingerprint(version, num_terms, terms));
}
*/
