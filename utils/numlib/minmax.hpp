#pragma once

#include "array.hpp"

namespace numlib {

template <typename InputArray, typename Comparator, typename OutputArray>
void ArgRelExtrema(InputArray input, OutputArray &output, Comparator comp, int order = 1) {
    size_t n = Size(input);
    for (size_t i = 0; i < n; i++) {
        auto current_value = input[i];
        int found = 0;
        for (size_t k = 1; k < static_cast<unsigned int>(order) + 1; k++) {
            if (k <= i) {
                if (!comp(current_value, input[i - k])) {
                    found |= 1;
                    break;
                }
            } else {
                found |= 1;
                break;
            }
            if (i + k < n) {
                if (!comp(current_value, input[i + k])) {
                    found |= 2;
                    break;
                }
            } else {
                found |= 2;
                break;
            }
        }
        if (found == 0) {
            output.Append(i);
        }
    }
}

template <typename InputArray, typename OutputArray>
void ArgRelMax(InputArray input, OutputArray &output, int order = 1) {
    ArgRelExtrema(input, output, [](auto a, auto b) { return a > b; }, order);
}

template <typename InputArray, typename OutputArray>
void ArgRelMin(InputArray input, OutputArray &output, int order = 1) {
    ArgRelExtrema(input, output, [](auto a, auto b) { return a < b; }, order);
}

template <typename InputArray, typename T = size_t>
Array<T> ArgRelMax(InputArray input, int order = 1) {
    Array<T> output(input.allocator());
    ArgRelMax(input, output, order);
    return output;
}

template <typename InputArray, typename T = size_t>
Array<T> ArgRelMin(InputArray input, int order = 1) {
    Array<T> output(input.allocator());
    ArgRelMin(input, output, order);
    return output;
}

}
