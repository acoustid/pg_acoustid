#include "fingerprint_matcher.h"
#include "common.h"

#include <math.h>
#include <string.h>

#define ALIGN_TERM_BITS 12
#define ALIGN_TERM_SHIFT (32 - ALIGN_TERM_BITS)
#define ALIGN_TERM_MASK ((1 << ALIGN_TERM_BITS) - 1)

#define ALIGN_OFFSET_BITS (32 - ALIGN_TERM_BITS - ALIGN_SOURCE_BITS)
#define ALIGN_OFFSET_SHIFT (ALIGN_TERM_SHIFT - ALIGN_OFFSET_BITS)
#define ALIGN_OFFSET_MASK ((1 << ALIGN_OFFSET_BITS) - 1)

#define ALIGN_SOURCE_BITS 1
#define ALIGN_SOURCE_SHIFT (ALIGN_OFFSET_SHIFT - ALIGN_SOURCE_BITS)
#define ALIGN_SOURCE_MASK ((1 << ALIGN_SOURCE_BITS) - 1)

#define MAX_FINGERPRINT_SIZE (1 << ALIGN_OFFSET_BITS) - 1

#define PACK_OFFSET(term, offset, source)                                                                              \
    ((((term)&ALIGN_TERM_MASK) << ALIGN_TERM_SHIFT) | (((offset)&ALIGN_OFFSET_MASK) << ALIGN_OFFSET_SHIFT) |           \
     (((source)&ALIGN_SOURCE_MASK) << ALIGN_SOURCE_SHIFT))

#define UNPACK_OFFSET_TERM(x) (((x) >> ALIGN_TERM_SHIFT) & ALIGN_TERM_MASK)
#define UNPACK_OFFSET_OFFSET(x) (((x) >> ALIGN_OFFSET_SHIFT) & ALIGN_OFFSET_MASK)
#define UNPACK_OFFSET_SOURCE(x) (((x) >> ALIGN_SOURCE_SHIFT) & ALIGN_SOURCE_MASK)

void fingerprint_matcher_init(FingerprintMatcherContext *ctx) {
    for (size_t i = 0; i < 2; i++) {
        ctx->terms[i] = NULL;
        ctx->num_terms[i] = 0;
    }
    ctx->results = NULL;
    ctx->num_results = 0;
}

void fingerprint_matcher_close(FingerprintMatcherContext *ctx) { (void)ctx; }

int fingerprint_matcher_load_fingerprint(FingerprintMatcherContext *ctx, int side, uint32_t *terms, size_t num_terms) {
    if (side < 0 || side > 1) {
        return -1;
    }
    if (num_terms == 0 || num_terms > MAX_FINGERPRINT_SIZE) {
        return -1;
    }
    ctx->terms[side] = terms;
    ctx->num_terms[side] = num_terms;
    return 0;
}

static int compare_uint32(const void *ap, const void *bp) {
    uint32_t a = *(uint32_t *)ap;
    uint32_t b = *(uint32_t *)bp;
    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    } else {
        return 0;
    }
}

void generate_gaussian_window(float *data, size_t size, float sigma) {
    float sigma2 = sigma * sigma;
    float sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        float x = (float)i - (float)size / 2.0;
        float y = exp(-x * x / (2.0 * sigma2));
        data[i] = y;
        sum += y;
    }
    for (size_t i = 0; i < size; i++) {
        data[i] /= sum;
    }
}

void convolve(const float *input_data, size_t size, const float *window_data, size_t window_size, float *output_data) {
    for (size_t i = 0; i < size; i++) {
        float sum = 0.0;
        for (size_t j = 0; j < window_size; j++) {
            float value;
            if (i + j < window_size / 2) {
                value = 0.0;
            } else {
                size_t k = i + j - window_size / 2;
                if (k >= size) {
                    value = 0.0;
                } else {
                    value = input_data[k];
                }
            }
            sum += value * window_data[j];
        }
        output_data[i] = sum;
    }
}

size_t align_fingerprints(const uint32_t *terms1, size_t num_terms1, const uint32_t *terms2, size_t num_terms2,
                          AlignmentResult *results, size_t max_results) {
    size_t num_offsets, cursor, histogram_size, gaussian_size, num_results;
    float *histogram, *tmp_histogram, *gaussian;
    uint32_t *offsets;

    CU_ASSERT(num_terms1 > 0);
    CU_ASSERT(num_terms1 <= MAX_FINGERPRINT_SIZE);
    CU_ASSERT(terms1 != NULL);

    CU_ASSERT(num_terms2 > 0);
    CU_ASSERT(num_terms2 <= MAX_FINGERPRINT_SIZE);
    CU_ASSERT(terms2 != NULL);

    num_offsets = (num_terms1 + num_terms2) * 3;
    offsets = CU_ALLOC(sizeof(offsets[0]) * num_offsets);

    cursor = 0;
    for (size_t i = 0; i < num_terms1; i++) {
        uint32_t term = terms1[i];
        for (int k = 0; k < 3; k++) {
            offsets[cursor++] = PACK_OFFSET(term, i, 0);
            term = term >> ((32 - ALIGN_TERM_BITS) / 2);
        }
    }
    for (size_t i = 0; i < num_terms2; i++) {
        uint32_t term = terms2[i];
        for (int k = 0; k < 3; k++) {
            offsets[cursor++] = PACK_OFFSET(term, i, 1);
            term = term >> ((32 - ALIGN_TERM_BITS) / 2);
        }
    }
    qsort(offsets, num_offsets, sizeof(offsets[0]), compare_uint32);

    histogram_size = num_terms1 + num_terms2;
    histogram = CU_ALLOC(sizeof(histogram[0]) * histogram_size);
    memset(histogram, 0, sizeof(histogram[0]) * histogram_size);

    for (size_t i = 0; i < num_offsets; i++) {
        uint32_t term = UNPACK_OFFSET_TERM(offsets[i]);
        uint32_t offset1 = UNPACK_OFFSET_OFFSET(offsets[i]);
        if (UNPACK_OFFSET_SOURCE(offsets[i]) != 0) {
            continue;
        }
        for (size_t j = i + 1; j < num_offsets; j++) {
            if (UNPACK_OFFSET_TERM(offsets[j]) != term) {
                break;
            }
            uint32_t offset2 = UNPACK_OFFSET_OFFSET(offsets[j]);
            if (UNPACK_OFFSET_SOURCE(offsets[j]) != 1) {
                continue;
            }
            const size_t offset_diff = offset1 + num_terms2 - offset2;
            histogram[offset_diff] += 1.0;
        }
    }

    gaussian_size = 16;
    gaussian = CU_ALLOC(sizeof(gaussian[0]) * gaussian_size);
    generate_gaussian_window(gaussian, gaussian_size, gaussian_size / 12.0);

    tmp_histogram = CU_ALLOC(sizeof(tmp_histogram[0]) * histogram_size);
    convolve(histogram, histogram_size, gaussian, gaussian_size, tmp_histogram);
    CU_FREE(histogram);
    histogram = tmp_histogram;

    /*    for (size_t i = 0; i < histogram_size; i++) {
            int offset = i - num_terms1;
            CU_LOG_INFO("histogram[%d] = %f %f", offset, histogram[i], histogram2[i]);
        }*/

    num_results = 0;
    for (size_t i = 1; i + 1 < histogram_size && num_results < max_results; i++) {
        if (histogram[i - 1] <= histogram[i] && histogram[i] >= histogram[i + 1]) {
            int offset = i - num_terms2;
            double score = histogram[i] * 2.0 / num_offsets;
            if (score > 0.1) {
                results[num_results].offset = offset;
                results[num_results].score = score;
                num_results++;
                CU_LOG_INFO("found possible match at offset %d with score %f", offset, score);
            }
        }
    }

    CU_FREE(gaussian);
    CU_FREE(histogram);
    CU_FREE(offsets);

    return num_results;
}

size_t compare_fingerprints(const uint32_t *terms1, size_t size1, const uint32_t *terms2, size_t size2, int offset)
{
    size_t size, gaussian_size;
    float *ber, *ber_f1, *ber_f2;
    float *gaussian;

    if (offset < 0) {
        CU_ASSERT(size2 >= (size_t)-offset);
        terms2 += -offset;
        size2 -= -offset;
    } else {
        CU_ASSERT(size1 >= (size_t)offset);
        terms1 += offset;
        size1 -= offset;
    }

    size = size1 < size2 ? size1 : size2;

    ber = CU_ALLOC(sizeof(ber[0]) * size);
    for (size_t i = 0; i < size; i++) {
        ber[i] = __builtin_popcount(terms1[i] ^ terms2[i]);
    }

    gaussian_size = 16;
    gaussian = CU_ALLOC(sizeof(gaussian[0]) * gaussian_size);

    ber_f1 = CU_ALLOC(sizeof(ber_f1[0]) * size);
    generate_gaussian_window(gaussian, gaussian_size, 3.6);
    convolve(ber, size, gaussian, gaussian_size, ber_f1);

    ber_f2 = CU_ALLOC(sizeof(ber_f2[0]) * size);
    generate_gaussian_window(gaussian, gaussian_size, 3.6);
    convolve(ber_f1, size, gaussian, gaussian_size, ber_f2);

    for (size_t i = 0; i < size; i++) {
        CU_LOG_INFO("ber[%ld] = %f %f %f %f", i, ber[i], ber_f1[i], ber_f2[i], ber_f1[i] - ber_f2[i]);
    }

    CU_FREE(ber);

    return 0;
}
