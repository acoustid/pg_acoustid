#include "pg_acoustid.h"

#include "utils/jsonb.h"
#include "utils/numeric.h"
#include "utils/fmgrprotos.h"
#include "catalog/pg_type.h"
#include "funcapi.h"
#include "miscadmin.h"

#include "utils/align_fingerprints.h"

PG_FUNCTION_INFO_V1(acoustid_align_fingerprints);

Datum
acoustid_align_fingerprints(PG_FUNCTION_ARGS)
{
    ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	Tuplestorestate *tupstore;
	HeapTuple tuple;
	TupleDesc tupdesc;
	MemoryContext per_query_ctx;
	MemoryContext oldcontext;

    ArrayType *fp1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *fp2 = PG_GETARG_ARRAYTYPE_P(1);
    size_t num_terms1, num_terms2, num_offsets;
    uint32_t *terms1, *terms2;
    int offsets[10], scores[10], max_offsets = 10, max_distance = -1;
    Datum values[2];
    bool isnull[2];

	/* check to see if caller supports us returning a tuplestore */
	if (!rsinfo || !(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("materialize mode required, but it is not "
						"allowed in this context")));

    num_terms1 = ARR_DIMS(fp1)[0];
    num_terms2 = ARR_DIMS(fp2)[0];

    terms1 = (uint32_t *) ARR_DATA_PTR(fp1);
    terms2 = (uint32_t *) ARR_DATA_PTR(fp2);

    num_offsets = AlignFingerprints(terms1, num_terms1, terms2, num_terms2, offsets, scores, max_offsets, max_distance);

   	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);

	/* get the requested return tuple description */
	tupdesc = CreateTupleDescCopy(rsinfo->expectedDesc);

	/*
	 * Check to make sure we have a reasonable tuple descriptor
	 */
	if (tupdesc->natts != 2 ||
		TupleDescAttr(tupdesc, 0)->atttypid != INT4OID ||
		TupleDescAttr(tupdesc, 1)->atttypid != INT4OID)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("query-specified return tuple and "
						"function return type are not compatible")));

	/* let the caller know we're sending back a tuplestore */
	rsinfo->returnMode = SFRM_Materialize;

	/* initialize our tuplestore */
	tupstore = tuplestore_begin_heap(true, false, work_mem);

    isnull[0] = false;
    isnull[1] = false;

	for (size_t i = 0; i < num_offsets; i++)
	{
        values[0] = Int32GetDatum(offsets[i]);
        values[1] = Int32GetDatum(scores[i]);

		tuple = heap_form_tuple(tupdesc, values, isnull);
		tuplestore_puttuple(tupstore, tuple);
	}

	tuplestore_donestoring(tupstore);
	rsinfo->setResult = tupstore;

	/*
	 * SFRM_Materialize mode expects us to return a NULL Datum. The actual
	 * tuples are in our tuplestore and passed back through rsinfo->setResult.
	 * rsinfo->setDesc is set to the tuple description that we actually used
	 * to build our tuples with, so the caller can verify we did what it was
	 * expecting.
	 */
	rsinfo->setDesc = tupdesc;
	MemoryContextSwitchTo(oldcontext); 

    PG_FREE_IF_COPY(fp1, 0);
    PG_FREE_IF_COPY(fp2, 1);

    PG_RETURN_NULL();
}
