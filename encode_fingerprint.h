#ifndef PG_ACOUSTID_ENCODE_FINGERPRINT_H
#define PG_ACOUSTID_ENCODE_FINGERPRINT_H

#include "acoustid.h"
#include "fingerprint.h"

extern ssize_t
get_encoded_fingerprint_size(FingerprintData* fp);
extern ssize_t
encode_fingerprint(FingerprintData* fp, uint8_t* output, size_t output_size);
extern FingerprintData*
decode_fingerprint(const uint8_t* input, size_t input_len);

#endif
