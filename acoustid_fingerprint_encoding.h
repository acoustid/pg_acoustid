#ifndef PG_ACOUSTID_FINGERPRINT_ENCODING_H_
#define PG_ACOUSTID_FINGERPRINT_ENCODING_H_

#include "common.h"

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_bytea(PG_FUNCTION_ARGS);

#endif // PG_ACOUSTID_FINGERPRINT_ENCODING_H_
