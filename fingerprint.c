#include "acoustid.h"

Fingerprint *create_empty_fingerprint(int num_terms)
{
    Fingerprint *fp;
    fp = palloc(FINGERPRINT_SIZE(num_terms));
    SET_VARSIZE(fp, FINGERPRINT_SIZE(num_terms));
    return fp;
}
