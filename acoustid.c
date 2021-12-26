/* acoustid_compare.c */

#include "postgres.h"
#include "fmgr.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(acoustid_compare);
Datum acoustid_compare(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_compare2);
Datum acoustid_compare2(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_compare3);
Datum acoustid_compare3(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(acoustid_extract_query);
Datum acoustid_extract_query(PG_FUNCTION_ARGS);
