#ifndef CHROMAPRINT_UTILS_COMMON_H_
#define CHROMAPRINT_UTILS_COMMON_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

#ifndef CU_LOG_DEBUG
#ifndef NDEBUG
#define CU_LOG_DEBUG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, "\n")
#else
#define CU_LOG_DEBUG(msg, ...)
#endif
#endif

#ifndef CU_LOG_INFO
#define CU_LOG_INFO(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, "\n")
#endif

#ifndef CU_LOG_ERROR
#define CU_LOG_ERROR(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, "\n")
#endif

#endif /* CHROMAPRINT_UTILS_COMMON_H_ */
