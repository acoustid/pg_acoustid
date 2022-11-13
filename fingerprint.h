#ifndef PG_ACOUSTID_FINGERPRINT_H
#define PG_ACOUSTID_FINGERPRINT_H

#include "acoustid.h"

typedef struct {
    int size;
    int version;
    uint32_t terms[];
} FingerprintData;

FingerprintData *fingerprint_data_alloc(int size);
FingerprintData *fingerprint_data_realloc(FingerprintData *fp, int size);
void fingerprint_data_free(FingerprintData *fp);

FingerprintData *fingerprint_data_from_string(const char *str);
char *fingerprint_data_to_string(FingerprintData *fp);

#endif
