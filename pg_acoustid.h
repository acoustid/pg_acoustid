#ifndef PG_ACOUSTID_H
#define PG_ACOUSTID_H

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"

/*
 * PostgreSQL type for storing compressed fingerprints.
 * It's a binary string stored as a bytea. It needs to be
 * decompressed for any operation.
 */
typedef bytea FingerprintType;

/*
 * PostgreSQL type for storing raw (uncompressed) fingerprints.
 */
typedef ArrayType RawFingerprintType;

#define PG_GETARG_FINGERPRINT_P(n)						\
    ((FingerprintType *) PG_GETARG_BYTEA_P(n))

#define PG_GETARG_FINGERPRINT_PP(n)						\
    ((FingerprintType *) PG_GETARG_BYTEA_PP(n))

#define PG_RETURN_FINGERPRINT_P(x)						\
    PG_RETURN_BYTEA_P(x)

/* Text input/output functions for acoustid_fingerprint */
extern Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS);
extern Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS);

extern Datum acoustid_fingerprint_extract_version(PG_FUNCTION_ARGS);
extern Datum acoustid_fingerprint_extract_terms(PG_FUNCTION_ARGS);

/* I/O functions for raw (uncompressed) fingerprints */
extern Datum acoustid_raw_fingerprint_in(PG_FUNCTION_ARGS);
extern Datum acoustid_raw_fingerprint_out(PG_FUNCTION_ARGS);

extern Datum acoustid_raw_fingerprint_extract_terms(PG_FUNCTION_ARGS);

extern Datum acoustid_align_fingerprints(PG_FUNCTION_ARGS);
extern Datum acoustid_match_fingerprints(PG_FUNCTION_ARGS);

#endif
