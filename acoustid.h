#ifndef PG_ACOUSTID_H_
#define PG_ACOUSTID_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#ifndef NO_PG_EXTENSION

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"

#else

#define palloc(s) malloc((s))
#define palloc0(s) calloc((s), 1)
#define repalloc(p, s) realloc((p), (s))
#define pfree(p) free((p))

#define elog(...)

#define FLEXIBLE_ARRAY_MEMBER /* no-op */

typedef struct {
    size_t len;
    size_t maxlen;
    size_t cursor;
    uint8_t *data;
} StringInfoData;

typedef StringInfoData *StringInfo;

#define PG_FUNCTION_ARGS /* no-op */

typedef void *Datum;

#define VARHDRSZ 4
#define VARSIZE(v) (*(uint32_t *)(v))
#define SET_VARSIZE(v, s) (*(uint32_t *)(v) = (s))

#endif

typedef struct {
    /* varlena header (do not touch directly!) */
    char vl_len_[4];

    /* fingerprint terms */
    uint32_t terms[FLEXIBLE_ARRAY_MEMBER];
} Fingerprint;

/* fmgr interface macros */
#define DatumGetFingerprintP(x) ((Fingerprint *)PG_DETOAST_DATUM(x))
#define PG_GETARG_FINGERPRINT_P(x) DatumGetFingerprintP(PG_GETARG_DATUM(x))
#define PG_RETURN_FINGERPRINT_P(x) PG_RETURN_POINTER(x)

/* fingerprint accessor macros */
#define FINGERPRINT_SIZE(nterms) (VARHDRSZ + sizeof(uint32_t) * (nterms))
#define FINGERPRINT_NTERMS(fp) ((VARSIZE(fp) - VARHDRSZ) / sizeof(uint32_t))
#define FINGERPRINT_TERM(fp, i) ((fp)->terms[i])

Fingerprint *create_empty_fingerprint(int nterms);

Fingerprint *create_fingerprint_from_str(char *str);
void fingerprint_to_str(Fingerprint *fp, StringInfoData *buf);

Fingerprint *create_fingerprint_from_bytes(StringInfoData *buf);
void fingerprint_to_bytes(Fingerprint *fp, StringInfoData *buf);

Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_recv(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_send(PG_FUNCTION_ARGS);

#endif  // PG_ACOUSTID_H_
