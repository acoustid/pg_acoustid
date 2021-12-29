/* acoustid_fingerprint_type.c */

#include "acoustid.h"

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
    fp = create_empty_fingerprint(num_terms);

    // Parse the fingerprint string and extract all terms
    for (it = str + 1, num_terms = 0; *it != '\0'; it++) {
        if (*it == ',') {
            it++;
        } else if (*it == '}') {
            break;
        }
        FINGERPRINT_TERM(fp, num_terms) = strtol(it, &it, 10);
        num_terms++;
    }

    // Update the size field to match the actual number of terms
    SET_VARSIZE(fp, FINGERPRINT_SIZE(num_terms));

    return fp;
}

void fingerprint_to_str(Fingerprint *fp, StringInfo buf) {
    int i, num_terms;
    appendStringInfo(buf, "{");
    num_terms = FINGERPRINT_NTERMS(fp);
    if (num_terms > 0) {
        appendStringInfo(buf, "%u", FINGERPRINT_TERM(fp, 0));
        for (i = 1; i < num_terms; i++) {
            appendStringInfo(buf, ",%u", FINGERPRINT_TERM(fp, i));
        }
    }
    appendStringInfo(buf, "}");
}

Fingerprint *create_fingerprint_from_bytes(StringInfo buf) {
    Fingerprint *fp;
    int i, num_terms;
    num_terms = pq_getmsgint(buf, sizeof(uint32));
    fp = create_empty_fingerprint(num_terms);
    for (i = 0; i < num_terms; i++) {
        FINGERPRINT_TERM(fp, i) = pq_getmsgint(buf, sizeof(uint32));
    }
    return fp;
}

void fingerprint_to_bytes(Fingerprint *fp, StringInfo buf) {
    int i, num_terms;
    num_terms = FINGERPRINT_NTERMS(fp);
    pq_sendint(buf, num_terms, 4);
    for (i = 0; i < num_terms; i++) {
        pq_sendint(buf, FINGERPRINT_TERM(fp, i), 4);
    }
}

Datum acoustid_fingerprint_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    Fingerprint *fp = create_fingerprint_from_str(str);
    PG_RETURN_FINGERPRINT_P(fp);
}

Datum acoustid_fingerprint_out(PG_FUNCTION_ARGS) {
    Fingerprint *fp = PG_GETARG_FINGERPRINT_P(0);
    StringInfoData buf;
    initStringInfo(&buf);
    fingerprint_to_str(fp, &buf);
    PG_RETURN_CSTRING(buf.data);
}

Datum acoustid_fingerprint_recv(PG_FUNCTION_ARGS) {
    StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
    Fingerprint *fp = create_fingerprint_from_bytes(buf);
    PG_RETURN_FINGERPRINT_P(fp);
}

Datum acoustid_fingerprint_send(PG_FUNCTION_ARGS) {
    Fingerprint *fp = PG_GETARG_FINGERPRINT_P(0);
    StringInfoData buf;
    pq_begintypsend(&buf);
    fingerprint_to_bytes(fp, &buf);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
