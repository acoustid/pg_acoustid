/* acoustid_fingerprint.c */

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"

#include "fingerprint_codec.h"

PG_FUNCTION_INFO_V1(acoustid_encode_fingerprint);
Datum       acoustid_encode_fingerprint(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_decode_fingerprint);
Datum       acoustid_decode_fingerprint(PG_FUNCTION_ARGS);

/*
 * Encodes an int4[] of hashes in Chromaprint's compressed format. The int4
 * values are taken as the unsigned 32-bit hashes they hold.
 */
Datum
acoustid_encode_fingerprint(PG_FUNCTION_ARGS)
{
	ArrayType *a = PG_GETARG_ARRAYTYPE_P(0);
	int32 version = PG_GETARG_INT32(1);
	size_t num_hashes, max_size, scratch_size, size = 0;
	uint8 *scratch;
	bytea *result;

	if (ARR_NDIM(a) > 1)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
				 errmsg("array must be one-dimensional")));
	if (ARR_HASNULL(a))
		ereport(ERROR,
				(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
				 errmsg("array must not contain nulls")));
	if (version < 0 || version > 255)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("fingerprint version must be between 0 and 255")));

	num_hashes = ArrayGetNItems(ARR_NDIM(a), ARR_DIMS(a));
	if (num_hashes > FPCODEC_MAX_HASHES)
		ereport(ERROR,
				(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
				 errmsg("fingerprint must not have more than %d hashes", FPCODEC_MAX_HASHES)));

	max_size = fpcodec_max_encoded_size(num_hashes);
	scratch_size = fpcodec_encode_scratch_size(num_hashes);
	result = (bytea *) palloc(VARHDRSZ + max_size);
	scratch = (uint8 *) palloc(scratch_size);
	if (fpcodec_encode((uint8) version, (const uint32 *) ARR_DATA_PTR(a), num_hashes,
					   (uint8 *) VARDATA(result), max_size, &size,
					   scratch, scratch_size) != FPCODEC_OK)
		elog(ERROR, "could not encode fingerprint");
	pfree(scratch);
	SET_VARSIZE(result, VARHDRSZ + size);

	PG_FREE_IF_COPY(a, 0);
	PG_RETURN_BYTEA_P(result);
}

static void
invalid_fingerprint(void)
{
	ereport(ERROR,
			(errcode(ERRCODE_INVALID_BINARY_REPRESENTATION),
			 errmsg("invalid fingerprint")));
}

/*
 * Decodes a fingerprint in Chromaprint's compressed format into an int4[]
 * of its hashes. The version byte is not returned.
 */
Datum
acoustid_decode_fingerprint(PG_FUNCTION_ARGS)
{
	bytea *input = PG_GETARG_BYTEA_PP(0);
	const uint8 *data = (const uint8 *) VARDATA_ANY(input);
	size_t len = VARSIZE_ANY_EXHDR(input);
	size_t num_hashes, nbytes, scratch_size;
	uint8 version, *scratch;
	ArrayType *result;

	if (fpcodec_decode_header(data, len, &version, &num_hashes) != FPCODEC_OK)
		invalid_fingerprint();

	if (num_hashes == 0) {
		PG_FREE_IF_COPY(input, 0);
		PG_RETURN_ARRAYTYPE_P(construct_empty_array(INT4OID));
	}

	nbytes = ARR_OVERHEAD_NONULLS(1) + num_hashes * sizeof(int32);
	result = (ArrayType *) palloc(nbytes);
	/* The decoder writes the data; the header and its padding are zeroed. */
	memset(result, 0, ARR_OVERHEAD_NONULLS(1));
	SET_VARSIZE(result, nbytes);
	result->ndim = 1;
	result->dataoffset = 0;
	result->elemtype = INT4OID;
	ARR_DIMS(result)[0] = (int) num_hashes;
	ARR_LBOUND(result)[0] = 1;

	scratch_size = fpcodec_decode_scratch_size(len, num_hashes);
	scratch = (uint8 *) palloc(scratch_size);
	if (fpcodec_decode(data, len, (uint32 *) ARR_DATA_PTR(result), num_hashes,
					   scratch, scratch_size) != FPCODEC_OK)
		invalid_fingerprint();
	pfree(scratch);

	PG_FREE_IF_COPY(input, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}
