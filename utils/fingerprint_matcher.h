#ifndef CHROMAPRINT_UTILS_FINGERPRINT_MATCHING_H_
#define CHROMAPRINT_UTILS_FINGERPRINT_MATCHING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int offset;
    double score;
} AlignmentResult;

typedef struct {
  double score;
} FingerprintMatcherResult;

typedef struct {
    uint32_t* terms[2];
    size_t num_terms[2];

    FingerprintMatcherResult *results;
    size_t num_results;
} FingerprintMatcherContext;

void fingerprint_matcher_init(FingerprintMatcherContext *ctx);

int fingerprint_matcher_load_fingerprint(FingerprintMatcherContext *ctx, int side, uint32_t *terms, size_t num_terms);

void fingerprint_matcher_close(FingerprintMatcherContext *ctx);

size_t align_fingerprints(const uint32_t *terms1, size_t size1, const uint32_t *terms2, size_t size2, AlignmentResult *results, size_t max_results);

size_t compare_fingerprints(const uint32_t *terms1, size_t size1, const uint32_t *terms2, size_t size2, int offset);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* CHROMAPRINT_UTILS_FINGERPRINT_MATCHING_H_ */
