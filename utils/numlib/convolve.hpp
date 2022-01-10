#pragma once

#include "base.hpp"

namespace numlib {

enum class ConvolveMode {
  FULL,
  SAME,
  VALID
};

template<typename InputType, typename KernelType, typename OutputType>
void Convolve(const InputType &in, const KernelType &kernel, OutputType &out, ConvolveMode mode = ConvolveMode::FULL) {
    size_t in_size = Size(in);
    size_t kernel_size = Size(kernel);
    size_t out_size, in_shift;
    switch (mode) {
        default:
        case ConvolveMode::FULL:
            out_size = in_size + kernel_size - 1;
            in_shift = 0;
            break;
        case ConvolveMode::SAME:
            out_size = in_size;
            in_shift = kernel_size / 2;
            break;
        case ConvolveMode::VALID:
            out_size = in_size - kernel_size + 1;
            in_shift = kernel_size - 1;
            break;
    }
    Resize(out, out_size);
    for (size_t i = 0; i < out_size; ++i) {
        out[i] = 0;
        for (size_t j = 0; j < kernel_size; ++j) {
            size_t k = i + in_shift;
            if (k >= j) {
                k = k - j;
                if (k < in_size) {
                    out[i] += in[k] * kernel[j];
                }
            }
        }
    }
}

template<typename InputType, typename KernelType>
InputType Convolve(const InputType &in, const KernelType &kernel, ConvolveMode mode = ConvolveMode::FULL) {
    InputType out;
    Convolve(in, kernel, out, mode);
    return out;
}

}  // namespace numlib
