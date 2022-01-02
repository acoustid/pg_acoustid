#ifndef CHROMAPRINT_UTILS_COMMON_H_
#define CHROMAPRINT_UTILS_COMMON_H_

#include <assert.h>
#include <stdlib.h>

#ifndef CU_ASSERT
#define CU_ASSERT(x) assert(x)
#endif

#ifndef CU_ALLOC
#define CU_ALLOC(x) malloc(x)
#endif

#ifndef CU_FREE
#define CU_FREE(x) free(x)
#endif

#ifndef CU_REALLOC
#define CU_REALLOC(x, y) realloc(x, y)
#endif

#endif /* CHROMAPRINT_UTILS_COMMON_H_ */
