/* acoustid_fingerprint_type.c */

#include "acoustid.h"

#include "encode_fingerprint.h"

PG_FUNCTION_INFO_V1(acoustid_fingerprint_type_in);
PG_FUNCTION_INFO_V1(acoustid_fingerprint_type_out);

/* "Methods" required for an expanded object */
static Size EF_get_flat_size(ExpandedObjectHeader *eohptr);
static void EF_flatten_into(ExpandedObjectHeader *eohptr, void *result,
							Size allocated_size);

static const ExpandedObjectMethods EF_methods = {EF_get_flat_size,
												 EF_flatten_into};

Size
EF_get_flat_size(ExpandedObjectHeader *eohptr)
{
	ExpandedFingerprintHeader *efh = (ExpandedFingerprintHeader *) eohptr;
	Size size;

	if (efh->flat_size)
	{
		size = efh->flat_size;
	}
	else
	{
		size = VARHDRSZ + get_encoded_fingerprint_size(efh->data);
		efh->flat_size = size;
	}

	elog(DEBUG5, "EF_get_flat_size %lu", size);
	return size;
}

void
EF_flatten_into(ExpandedObjectHeader *eohptr, void *result, Size allocated_size)
{
	ExpandedFingerprintHeader *efh = (ExpandedFingerprintHeader *) eohptr;
	Size encoded_size, size;

	elog(DEBUG5, "EF_flatten_into");
	encoded_size = encode_fingerprint(efh->data, (uint8_t *) VARDATA(result),
									  allocated_size - VARHDRSZ);
	size = encoded_size + VARHDRSZ;
	SET_VARSIZE(result, size);
	efh->flat_size = size;
}

Datum
make_expanded_fingerprint(FingerprintData *data, MemoryContext parentcontext)
{
	ExpandedFingerprintHeader *efh;
	MemoryContext objcxt;

	/*
	 * Allocate private context for expanded object.  We use a regular-size
	 * context, not a small one, because most fingerprints will be above
	 * 1k bytes.
	 */
	objcxt = AllocSetContextCreate(parentcontext, "expanded record",
								   ALLOCSET_DEFAULT_SIZES);

	/* Set up expanded fingerprint header */
	efh = (ExpandedFingerprintHeader *) MemoryContextAlloc(
		objcxt, sizeof(ExpandedFingerprintHeader));

	EOH_init_header(&efh->hdr, &EF_methods, objcxt);
	efh->ef_magic = EF_MAGIC;

	efh->flat_size = 0;
	efh->data = data;

	return EOHPGetRODatum(&efh->hdr);
}

Datum acoustid_fingerprint_type_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	FingerprintData *fp = fingerprint_data_from_string(str);
	PG_RETURN_DATUM(make_expanded_fingerprint(fp, CurrentMemoryContext));
}

Datum acoustid_fingerprint_type_out(PG_FUNCTION_ARGS)
{
	FingerprintType *input = PG_GETARG_FINGERPRINT_P(0);
	FingerprintData *fp;
	char *result;
	uint8_t *encoded_data;
	size_t encoded_size;

	encoded_data = (uint8_t *) VARDATA(input);
	encoded_size = VARSIZE(input) - VARHDRSZ;

	fp = decode_fingerprint(encoded_data, encoded_size);
	result = fingerprint_data_to_string(fp);

	PG_RETURN_CSTRING(result);
}
