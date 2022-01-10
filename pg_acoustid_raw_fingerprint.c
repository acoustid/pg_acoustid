#include "pg_acoustid.h"

#include "lib/stringinfo.h"
#include "catalog/pg_type.h"

PG_FUNCTION_INFO_V1(acoustid_raw_fingerprint_in);
PG_FUNCTION_INFO_V1(acoustid_raw_fingerprint_out);
PG_FUNCTION_INFO_V1(acoustid_raw_fingerprint_extract_terms);

Datum
acoustid_raw_fingerprint_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0), *it, *end;
    ArrayType *terms_array;
    uint32_t *terms;
    size_t num_terms, terms_array_size;
    int found_term;

    // Validate the fingerprint string and also count the number of terms in it
    if (str[0] != '{') {
        elog(ERROR, "Fingerprint string must start with '{'");
    }
    num_terms = 0;
    found_term = 0;
    for (it = str + 1;; it++) {
        if (*it == ',') {
            num_terms++;
            found_term = 0;
        } else if (*it == '}') {
            if (found_term) {
                num_terms++;
            }
            break;
        } else if (*it == '\0') {
            elog(ERROR, "Fingerprint string must end with '}'");
        } else if (*it == ' ') {
            elog(ERROR, "Fingerprint string must not contain spaces");
        } else if (*it < '0' || *it > '9') {
            elog(ERROR, "Fingerprint string must contain only digits");
        } else {
            found_term = 1;
        }
    }

    // Allocate the fingerprint structure
    terms_array_size = ARR_OVERHEAD_NONULLS(1) + sizeof(uint32_t) * num_terms;
    terms_array = (ArrayType *) palloc(terms_array_size);

    SET_VARSIZE(terms_array, terms_array_size);
    terms_array->ndim = 1;
    terms_array->dataoffset = 0;
    terms_array->elemtype = INT4OID;
    ARR_DIMS(terms_array)[0] = num_terms;
    ARR_LBOUND(terms_array)[0] = 1;

    terms = (uint32_t *) ARR_DATA_PTR(terms_array);

    // Parse the fingerprint string and extract all terms
    for (it = str + 1, num_terms = 0; *it != '\0';) {
        if (*it == ',') {
            it++;
        } else if (*it == '}') {
            break;
        }
        terms[num_terms] = strtol(it, &end, 10);
        it = end;
        num_terms++;
    }

    PG_RETURN_ARRAYTYPE_P(terms_array);
}

Datum
acoustid_raw_fingerprint_out(PG_FUNCTION_ARGS)
{
    ArrayType *terms_array = PG_GETARG_ARRAYTYPE_P(0);
    uint32_t *terms = (uint32_t *) ARR_DATA_PTR(terms_array);
    size_t num_terms = ARR_DIMS(terms_array)[0];

    // Render comma-separated list of terms
    StringInfoData buf;
    initStringInfo(&buf);
    appendStringInfo(&buf, "{");
    for (size_t i = 0; i < num_terms; i++) {
        appendStringInfo(&buf, "%u", terms[i]);
        if (i < num_terms - 1) {
            appendStringInfo(&buf, ",");
        }
    }
    appendStringInfo(&buf, "}");

    PG_RETURN_CSTRING(buf.data);
}

Datum
acoustid_raw_fingerprint_extract_terms(PG_FUNCTION_ARGS)
{
    ArrayType *terms_array = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *terms_array_out;

    // Just copy the input array
    terms_array_out = (ArrayType *) palloc(VARSIZE(terms_array));
    memcpy(terms_array_out, terms_array, VARSIZE(terms_array));
    PG_RETURN_ARRAYTYPE_P(terms_array_out);
}
