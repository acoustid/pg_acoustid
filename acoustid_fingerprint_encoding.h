#ifndef PG_ACOUSTID_FINGERPRINT_ENCODING_H_
#define PG_ACOUSTID_FINGERPRINT_ENCODING_H_

#include "acoustid_fingerprint_type.h"

Fingerprint *decode_fingerprint(const unsigned char *input, int input_len, int *version);
void encode_fingerprint(Fingerprint *fp, uint8_t **output, size_t *output_len);

Datum acoustid_fingerprint_encode_to_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_encode_to_bytea(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_text(PG_FUNCTION_ARGS);
Datum acoustid_fingerprint_decode_from_bytea(PG_FUNCTION_ARGS);

#endif // PG_ACOUSTID_FINGERPRINT_ENCODING_H_
