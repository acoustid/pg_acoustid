/* acoustid_chromaprint_encoding.c */

#include "postgres.h"
#include "fmgr.h"
#include "lib/stringinfo.h"

PG_FUNCTION_INFO_V1(chromaprint_in);
PG_FUNCTION_INFO_V1(chromaprint_out);
PG_FUNCTION_INFO_V1(chromaprint_recv);
PG_FUNCTION_INFO_V1(chromaprint_send);

Datum chromaprint_in(PG_FUNCTION_ARGS);
Datum chromaprint_out(PG_FUNCTION_ARGS);
Datum chromaprint_recv(PG_FUNCTION_ARGS);
Datum chromaprint_send(PG_FUNCTION_ARGS);

struct chromaprint {
    int length;
    uint32 *data;
};

// Parse comma-separated list of 32-bit integers
static struct chromaprint *chromaprint_from_str(char *str) {
    struct chromaprint *fp = palloc0(sizeof(struct chromaprint));
    char *p = str;
    int i = 0;
    while (*p) {
        char *end;
        uint32 val = strtoul(p, &end, 10);
        if (end == p) {
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                     errmsg("invalid input syntax for type chromaprint: \"%s\"", str)));
        }
        if (i >= fp->length) {
            fp->length = i + 1;
            fp->data = repalloc(fp->data, fp->length * sizeof(uint32 *));
        }
        fp->data[i++] = val;
        p = end;
        if (*p == ',') {
            p++;
        }
    }
    return fp;
}

static char *chromaprint_to_str(struct chromaprint *fp) {
    StringInfoData buf;
    initStringInfo(&buf);
    for (int i = 0; i < fp->length; i++) {
        if (i > 0) {
            appendStringInfoChar(&buf, ',');
        }
        appendStringInfo(&buf, "%u", fp->data[i]);
    }
    return buf.data;
}

#define DatumGetChromaprint(X)	    ((struct chromaprint *) DatumGetPointer(X))
#define ChromaprintGetDatum(X)	    PointerGetDatum(X)
#define PG_GETARG_CHROMAPRINT(n)    DatumGetChromaprint(PG_GETARG_DATUM(n))
#define PG_RETURN_CHROMAPRINT(x)	return ChromaprintGetDatum(x)

Datum chromaprint_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    PG_RETURN_CHROMAPRINT(chromaprint_from_str(str));
}

Datum chromaprint_out(PG_FUNCTION_ARGS) {
    struct chromaprint *fp = PG_GETARG_CHROMAPRINT(0);
    PG_RETURN_CSTRING(chromaprint_to_str(fp));
}
