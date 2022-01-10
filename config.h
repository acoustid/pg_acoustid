#ifndef CONFIG_H
#define CONFIG_H

#include "postgres.h"

#ifdef Max
#undef Max
#endif

#ifdef Min
#undef Min
#endif

#define CU_ALLOC(x) palloc(x)
#define CU_FREE(x) pfree(x)
#define CU_REALLOC(x,y) repalloc(x,y)
#define CU_ASSERT(x) Assert(x)

#endif
