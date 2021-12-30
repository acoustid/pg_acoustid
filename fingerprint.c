#include "acoustid.h"

FingerprintData *fingerprint_data_alloc(int size)
{
    FingerprintData *fp = palloc0(sizeof(FingerprintData) + size * sizeof(uint32_t));
    fp->size = size;
    return fp;
}

void fingerprint_data_free(FingerprintData *fp) {
    pfree(fp);
}

// Parse comma-separated list of 32-bit integers enclosed in braces, e.g.
// "{1,2,3}"
FingerprintData *fingerprint_data_from_string(const char *str) {
    FingerprintData *fp;
    const char *it;
    char *end;
    int num_terms, found_term;

    // Validate the fingerprint string and also count the number of terms in it
    if (str[0] != '{') {
        elog(ERROR, "Fingerprint string must start with '{'");
        return NULL;
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
            return NULL;
        } else if (*it == ' ') {
            elog(ERROR, "Fingerprint string must not contain spaces");
        } else if (*it < '0' || *it > '9') {
            elog(ERROR, "Fingerprint string must contain only digits");
        } else {
            found_term = 1;
        }
    }

    // Allocate the fingerprint structure
    fp = fingerprint_data_alloc(num_terms);

    // Parse the fingerprint string and extract all terms
    for (it = str + 1, num_terms = 0; *it != '\0';) {
        if (*it == ',') {
            it++;
        } else if (*it == '}') {
            it++;
            if (*it == 'v') {
                it++;
                fp->version = strtol(it, &end, 10);
                it = end;
            }
            break;
        }
        fp->terms[num_terms] = strtol(it, &end, 10);
        it = end;
        num_terms++;
    }

    return fp;
}

char *fingerprint_data_to_string(FingerprintData *fp) {
    StringInfoData buf;
    int i;

    initStringInfo(&buf);

    if (fp->size == 0) {
        appendStringInfo(&buf, "{}");
    } else {
        appendStringInfo(&buf, "{%u", fp->terms[0]);
        for (i = 1; i < fp->size; i++) {
            appendStringInfo(&buf, ",%u", fp->terms[i]);
        }
        appendStringInfo(&buf, "}");
    }

    if (fp->version != 0) {
        appendStringInfo(&buf, "v%u", fp->version);
    }

    return buf.data;
}
