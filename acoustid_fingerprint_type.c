/* acoustid_fingerprint_type.c */

#include "acoustid_fingerprint_type.h"

#include "libpq/pqformat.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_in);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_out);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_recv);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_send);

// Parse comma-separated list of 32-bit integers enclosed in braces, e.g.
// "{1,2,3}"
Fingerprint *create_fingerprint_from_str(char *str) {
    Fingerprint *fp;
    char *it;
    int num_terms;

    if (str[0] != '{') {
        elog(ERROR, "Fingerprint string must start with '{'");
    }

    // Count the number of terms in the fingerprint string
    num_terms = 0;
    for (it = str + 1;; it++) {
        if (*it == ',') {
            num_terms++;
        } else if (*it == '}') {
            num_terms++;
            break;
        } else if (*it == '\0') {
            elog(ERROR, "Fingerprint string must end with '}'");
        }
    }

    // Allocate the fingerprint structure
    fp = palloc(VARHDRSZ + sizeof(uint32) * num_terms);

    // Parse the fingerprint string and extract all terms
    for (it = str + 1, num_terms = 0; *it != '\0'; it++) {
        if (*it == ',') {
            it++;
        } else if (*it == '}') {
            break;
        }
        fp->data[num_terms++] = strtoul(it, &it, 10);
    }

    // Update the size field to match the actual number of terms
    SET_VARSIZE(fp, VARHDRSZ + sizeof(uint32) * num_terms);

    return fp;
}

void fingerprint_to_str(Fingerprint *fp, StringInfo buf) {
    int i, num_terms;
    appendStringInfo(buf, "{");
    num_terms = VARSIZE(fp) / sizeof(uint32);
    if (num_terms > 0) {
        appendStringInfo(buf, "%u", fp->data[0]);
        for (i = 1; i < num_terms; i++) {
            appendStringInfo(buf, ",%u", fp->data[i]);
        }
    }
    appendStringInfo(buf, "}");
}

Fingerprint *create_fingerprint_from_bytes(StringInfo buf) {
    Fingerprint *fp;
    int i, num_terms;
    num_terms = pq_getmsgint(buf, sizeof(uint32));
    fp = palloc(VARHDRSZ + sizeof(uint32) * num_terms);
    SET_VARSIZE(fp, VARHDRSZ + sizeof(uint32) * num_terms);
    for (i = 0; i < num_terms; i++) {
        fp->data[i] = pq_getmsgint(buf, sizeof(uint32));
    }
    return fp;
}

void fingerprint_to_bytes(Fingerprint *fp, StringInfo buf) {
    int i, num_terms;
    num_terms = VARSIZE(fp) / sizeof(uint32);
    pq_sendint32(buf, num_terms);
    for (i = 0; i < num_terms; i++) {
        pq_sendint32(buf, fp->data[i]);
    }
}

Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    Fingerprint *fp = create_fingerprint_from_str(str);
    PG_RETURN_POINTER(fp);
}

Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS) {
    Fingerprint *fp = (Fingerprint *)PG_GETARG_POINTER(0);
    StringInfoData buf;
    initStringInfo(&buf);
    fingerprint_to_str(fp, &buf);
    PG_RETURN_CSTRING(buf.data);
}

Datum acoustid_fingerprint_recv(PG_FUNCTION_ARGS) {
    StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
    Fingerprint *fp = create_fingerprint_from_bytes(buf);
    PG_RETURN_POINTER(fp);
}

Datum acoustid_fingerprint_send(PG_FUNCTION_ARGS) {
    Fingerprint *fp = (Fingerprint *)PG_GETARG_POINTER(0);
    StringInfoData buf;
    pq_begintypsend(&buf);
    fingerprint_to_bytes(fp, &buf);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
