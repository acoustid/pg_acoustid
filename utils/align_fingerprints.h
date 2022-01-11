#pragma once

#include <stdint.h>
#include <stddef.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t AlignFingerprints(
    const uint32_t *terms1, size_t num_terms1,
    const uint32_t *terms2, size_t num_terms2,
    int *offsets, float *scores, size_t max_offsets,
    int max_distance
);

#ifdef __cplusplus
}
#endif  
