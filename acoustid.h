#ifndef PG_ACOUSTID_H_
#define PG_ACOUSTID_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#ifndef NO_PG_EXTENSION
#include "postgres.h"
#include "fmgr.h"
#include "utils/expandeddatum.h"
#include "libpq/pqformat.h"
#else
#include "mock_postgres.h"
#endif

#include "fingerprint.h"

/*
 * Packed fingerprint, implemented as PostgreSQL varlena object.
 *
 * The first 4 bytes contain the total object size in bytes. Be sure to use
 * VARSIZE() and SET_VARSIZE() to access it.
 *
 * The remaining bytes contain the fingerprint data.
 */
typedef struct {
    /* varlena header (do not touch directly!) */
    char vl_len_[4];

    /* fingerprint data */
    uint8_t data[FLEXIBLE_ARRAY_MEMBER];
} FingerprintType;

/*
 * An expanded fingerprint is contained within a private memory context (as
 * all expanded objects must be) and has a control structure as below.
 *
 * The expanded fingerprint might contain a regular packed fingerprint if
 * that was the original input. Otherwise, it contains the fingerprint data
 * as an array of uint32_t values.
 */
#define EP_MAGIC 922322027

typedef struct ExpandedFingerprintHeader {
     /* Standard header for expanded objects */
     ExpandedObjectHeader hdr;

     /*
      * flat_size is the current space requirement for the flat equivalent of
      * the expanded array, if known; otherwise it's 0.  We store this to make
      * consecutive calls of get_flat_size cheap.
      */
     Size flat_size;

     /*
      * fdata points to the flat representation if it is valid, else it is
      * NULL.   
      */
     FingerprintType *fdata;

     /*
      * data is the expanded fingerprint data.
      */
     FingerprintData *data;
} ExpandedFingerprintHeader;

typedef union AnyFingerprintType {
    FingerprintType fp;
    ExpandedFingerprintHeader efp;
} AnyFingerprintType;

#define PG_GETARG_FINGERPRINT_P(x) ((FingerprintType *) PG_DETOAST_DATUM(PG_GETARG_DATUM(x)))
#define PG_RETURN_FINGERPRINT_P(x) PG_RETURN_POINTER(x)

Datum acoustid_fingerprint_type_in(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_type_out(PG_FUNCTION_ARGS);

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_bytea(PG_FUNCTION_ARGS);

#endif  // PG_ACOUSTID_H_
