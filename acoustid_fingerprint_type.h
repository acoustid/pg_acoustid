#ifndef PG_ACOUSTID_FINGERPRINT_TYPE_H_
#define PG_ACOUSTID_FINGERPRINT_TYPE_H_

#include "postgres.h"
#include "fmgr.h"
#include "lib/stringinfo.h"

typedef struct {
    int32 size;
    uint32 data[0];
} Fingerprint;

Fingerprint *create_fingerprint_from_str(char *str);
void fingerprint_to_str(Fingerprint *fp, StringInfo buf);

Fingerprint *create_fingerprint_from_bytes(StringInfo buf);
void fingerprint_to_bytes(Fingerprint *fp, StringInfo buf);

Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_recv(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_send(PG_FUNCTION_ARGS);

#endif // PG_ACOUSTID_FINGERPRINT_TYPE_H_
