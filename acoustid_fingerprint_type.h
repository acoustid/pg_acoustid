#ifndef PG_ACOUSTID_FINGERPRINT_TYPE_H_
#define PG_ACOUSTID_FINGERPRINT_TYPE_H_

#include "postgres.h"
#include "fmgr.h"
#include "lib/stringinfo.h"

typedef struct {
    /* varlena header (do not touch directly!) */
    char vl_len_[4];

    /* fingerprint terms */
    uint32 terms[FLEXIBLE_ARRAY_MEMBER];
} Fingerprint;

/* fmgr interface macros */
#define DatumGetFingerprintP(x) ((Fingerprint *)PG_DETOAST_DATUM(x))
#define PG_GETARG_FINGERPRINT_P(x) DatumGetFingerprintP(PG_GETARG_DATUM(x))
#define PG_RETURN_FINGERPRINT_P(x) PG_RETURN_POINTER(x)

/* fingerprint accessor macros */
#define FINGERPRINT_SIZE(nterms) (VARHDRSZ + sizeof(uint32) * (nterms))
#define FINGERPRINT_NTERMS(fp) ((VARSIZE(fp) - VARHDRSZ) / sizeof(uint32))
#define FINGERPRINT_TERM(fp, i) ((fp)->terms[i])

Fingerprint *create_empty_fingerprint(int nterms);

Fingerprint *create_fingerprint_from_str(char *str);
void fingerprint_to_str(Fingerprint *fp, StringInfo buf);

Fingerprint *create_fingerprint_from_bytes(StringInfo buf);
void fingerprint_to_bytes(Fingerprint *fp, StringInfo buf);

Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_recv(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_send(PG_FUNCTION_ARGS);

#endif // PG_ACOUSTID_FINGERPRINT_TYPE_H_
