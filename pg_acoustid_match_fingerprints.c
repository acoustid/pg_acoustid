#include "pg_acoustid.h"

#include "utils/fmgrprotos.h"
#include "catalog/pg_type.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "access/htup_details.h"

PG_FUNCTION_INFO_V1(acoustid_match_fingerprints);

Datum
acoustid_match_fingerprints(PG_FUNCTION_ARGS)
{
    ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	Tuplestorestate *tupstore;
	HeapTuple tuple;
	TupleDesc tupdesc;
	MemoryContext per_query_ctx;
	MemoryContext oldcontext;
    Datum values[4];
    bool nulls[4];

    ArrayType *fp1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *fp2 = PG_GETARG_ARRAYTYPE_P(1);
    int offset = PG_GETARG_INT32(2);
   
    size_t nresults = 1;
        
    size_t nterms1 = ARR_DIMS(fp1)[0];
    size_t nterms2 = ARR_DIMS(fp2)[0];

    uint32_t *terms1 = (uint32_t *) ARR_DATA_PTR(fp1);
    uint32_t *terms2 = (uint32_t *) ARR_DATA_PTR(fp2);

	/* check to see if caller supports us returning a tuplestore */
	if (!rsinfo || !(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("materialize mode required, but it is not "
						"allowed in this context")));

    (void) nterms1;
    (void) nterms2;
    (void) terms1;
    (void) terms2;
    (void) offset;

   	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);

	/* get the requested return tuple description */
	tupdesc = CreateTupleDescCopy(rsinfo->expectedDesc);

	/*
	 * Check to make sure we have a reasonable tuple descriptor
	 */
	if (tupdesc->natts != 4 ||
		TupleDescAttr(tupdesc, 0)->atttypid != INT4OID ||
		TupleDescAttr(tupdesc, 1)->atttypid != INT4OID ||
		TupleDescAttr(tupdesc, 2)->atttypid != INT4OID ||
		TupleDescAttr(tupdesc, 3)->atttypid != FLOAT4OID)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("query-specified return tuple and "
						"function return type are not compatible")));

	/* let the caller know we're sending back a tuplestore */
	rsinfo->returnMode = SFRM_Materialize;

	/* initialize our tuplestore */
	tupstore = tuplestore_begin_heap(true, false, work_mem);

    nulls[0] = false;
    nulls[1] = false;
    nulls[2] = false;
    nulls[3] = false;

	for (size_t i = 0; i < nresults; i++)
	{
        values[0] = Int32GetDatum(0);
        values[1] = Int32GetDatum(0);
        values[2] = Int32GetDatum(0);
        values[3] = Float4GetDatum(0.0);

		tuple = heap_form_tuple(tupdesc, values, nulls);
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

