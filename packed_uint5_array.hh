// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_UTILS_PACKED_UINT5_ARRAY_HH_
#define CHROMAPRINT_UTILS_PACKED_UINT5_ARRAY_HH_

#include <vector>

#include "common.h"
#include "packed_uint5_array.h"

inline void pack_uint5_array(const std::vector<uint8_t> &input, std::vector<uint8_t> &output) {
    output.resize(get_packed_uint5_array_size(input.size()));
    auto output_size = pack_uint5_array(input.data(), input.size(), output.data(), output.size());
    CU_ASSERT(output_size <= output.size());
    output.resize(output_size);
}

inline void unpack_uint5_array(const std::vector<uint8_t> &input, std::vector<uint8_t> &output) {
    output.resize(get_unpacked_uint5_array_size(input.size()));
    auto output_size = unpack_uint5_array(input.data(), input.size(), output.data(), output.size());
    CU_ASSERT(output_size <= output.size());
    output.resize(output_size);
}

inline std::vector<uint8_t> pack_uint5_array(const std::vector<uint8_t> &input) {
    std::vector<uint8_t> output;
    pack_uint5_array(input, output);
    return output;
}

inline std::vector<uint8_t> unpack_uint5_array(const std::vector<uint8_t> &input) {
    std::vector<uint8_t> output;
    unpack_uint5_array(input, output);
    return output;
}

#endif
