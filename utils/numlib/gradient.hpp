#pragma once

#include "base.hpp"

namespace numlib {

template <typename InT, typename OutT>
void Gradient(const InT& in, OutT& out) {
    size_t size = Size(in);
    out.resize(in.size());
    if (size == 1) {
        out[0] = 0;
    } else if (size >= 2) {
        out[0] = in[1] - in[0];
        for (size_t i = 1; i + 1 < size; ++i) {
            out[i] = 0.5 * (in[i + 1] - in[i - 1]);
        }
        out[size - 1] = in[size - 1] - in[size - 2];
    }
}

template<typename InputVector>
InputVector Gradient(const InputVector &in) {
    InputVector out;
    Gradient(in, out);
    return out;
}

} // namespace numlib
