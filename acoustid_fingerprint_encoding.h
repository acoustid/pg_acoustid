#ifndef PG_ACOUSTID_FINGERPRINT_ENCODING_H_
#define PG_ACOUSTID_FINGERPRINT_ENCODING_H_

#include "acoustid_fingerprint_type.h"

Fingerprint *decode_fingerprint(StringInfo buf, int *version);
void encode_fingerprint(Fingerprint *fp, StringInfo buf);

Datum acoustid_fingerprint_encode(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode(PG_FUNCTION_ARGS);

#endif // PG_ACOUSTID_FINGERPRINT_ENCODING_H_
