#include "common.h"
#include "align_fingerprints.h"
#include "numlib/array.hpp"
#include "numlib/convolve.hpp"
#include "numlib/minmax.hpp"
#include "numlib/window.hpp"

namespace nb = numlib;

inline uint32_t PackTermAndOffset(uint16_t term, uint16_t offset) {
    return ((uint32_t)term << 16) | offset;
}

inline bool TermEq(uint32_t a, uint32_t b) {
    return ((a ^ b) & 0xFFFF0000) == 0;
}

inline uint16_t UnpackTerm(uint32_t packed) {
    return (uint16_t)((packed >> 16) & 0xFFFF);
}

inline uint16_t UnpackOffset(uint32_t packed) {
    return (uint16_t)(packed & 0xFFFF);
}

nb::Array<uint32_t> ExpandTerms(const uint32_t *terms, size_t num_terms) {
    nb::Array<uint32_t> expanded_terms;
    expanded_terms.reserve(num_terms * 2);
    for (size_t i = 0; i < num_terms; ++i) {
        expanded_terms.push_back(PackTermAndOffset(terms[i], i));
        expanded_terms.push_back(PackTermAndOffset(terms[i] >> 16, i));
    }
    std::sort(expanded_terms.begin(), expanded_terms.end());
    return expanded_terms;
}

void MergeExpandedTerms(const nb::Array<uint32_t> &terms1, const nb::Array<uint32_t> &terms2, nb::Array<float> &offset_counts, size_t center) {
    size_t i1 = 0, i2 = 0;
    while (i1 < terms1.size() && i2 < terms2.size()) {
        if (TermEq(terms1[i1], terms2[i2])) {
            size_t matching1 = 1;
            size_t matching2 = 1;
            while (i1 + matching1 < terms1.size() && TermEq(terms1[i1 + matching1], terms1[i1])) {
                ++matching1;
            }
            while (i2 + matching2 < terms2.size() && TermEq(terms2[i2 + matching2], terms2[i2])) {
                ++matching2;
            }
            for (size_t j1 = 0; j1 < matching1; ++j1) {
                for (size_t j2 = 0; j2 < matching2; ++j2) {
                    auto offset = center + UnpackOffset(terms1[i1 + j1]) - UnpackOffset(terms2[i2 + j2]);
                    offset_counts[offset] += 1.0 / log(matching1 + matching2);
                }
            }
            i1 += matching1;
            i2 += matching2;
        } else if (terms1[i1] < terms2[i2]) {
            ++i1;
        } else {
            ++i2;
        }
    }
}

size_t AlignFingerprintsImpl(
    const uint32_t *terms1, size_t num_terms1,
    const uint32_t *terms2, size_t num_terms2,
    int *offsets, float *scores, size_t max_offsets,
    int max_distance)
{
    auto expanded_terms1 = ExpandTerms(terms1, num_terms1);
    auto expanded_terms2 = ExpandTerms(terms2, num_terms2);

    // Generate a histogram of the matching offset differences between the two fingerprints.
    auto offset_center = num_terms2;
    auto offset_counts = nb::Zeros<float>(num_terms1 + num_terms2);
    MergeExpandedTerms(expanded_terms1, expanded_terms2, offset_counts, offset_center);

    // Smooth the histogram using a short Gaussian window.
    auto kernel = nb::GaussianWindow(9, 0.6);
    auto smooth_offset_counts = nb::Convolve(offset_counts, kernel, nb::ConvolveMode::SAME);

    // Zero all values below the threshold, which is either 400% the mean or constant 9.0.
    auto threshold = nb::Max(9.0, nb::Mean(smooth_offset_counts) * 4.0);
    for (size_t i = 0; i < smooth_offset_counts.size(); ++i) {
        if (smooth_offset_counts[i] < threshold) {
            smooth_offset_counts[i] = 0.0;
        }
    }

    auto peaks = nb::ArgRelMax(smooth_offset_counts, 5);

    std::sort(peaks.begin(), peaks.end(), [&smooth_offset_counts](size_t i1, size_t i2) {
        return smooth_offset_counts[i1] > smooth_offset_counts[i2];
    });

    size_t num_offsets = 0;
    for (size_t j = 0; j < peaks.size(); ++j) {
        if (num_offsets >= max_offsets) {
            break;
        }
        auto i = peaks[j];
        int offset = i - offset_center;
        if (max_distance >= 0 && abs(offset) > max_distance) {
            continue;
        }
        offsets[num_offsets] = offset;
        scores[num_offsets] = smooth_offset_counts[i];
        num_offsets += 1;
    }
    return num_offsets;
}

size_t AlignFingerprints(
    const uint32_t *terms1, size_t num_terms1,
    const uint32_t *terms2, size_t num_terms2,
    int *offsets, float *scores, size_t max_offsets,
    int max_distance) {
    try {
        return AlignFingerprintsImpl(terms1, num_terms1, terms2, num_terms2, offsets, scores, max_offsets, max_distance);
    } catch (...) {
        return 0;
    }
}
