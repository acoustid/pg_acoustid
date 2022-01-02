// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_UTILS_BASE64_HH_
#define CHROMAPRINT_UTILS_BASE64_HH_

#include <string>
#include <vector>

#include "base64.h"
#include "common.h"

inline bool base64_validate(const std::string &input) { return base64_validate(input.data(), input.size()) == 0; }

inline void base64_encode(const std::vector<uint8_t> &input, std::string &output) {
    output.resize(get_base64_encoded_size(input.size()));
    auto output_size = base64_encode(input.data(), input.size(), &output[0], output.size(), 0);
    CU_ASSERT(output_size <= output.size());
    output.resize(output_size);
}

inline void base64_decode(const std::string &input, std::vector<uint8_t> &output) {
    output.resize(get_base64_decoded_size(input.size()));
    auto output_size = base64_decode(input.data(), input.size(), &output[0], output.size());
    CU_ASSERT(output_size <= output.size());
    output.resize(output_size);
}

inline std::string base64_encode(const std::vector<uint8_t> &input) {
    std::string output;
    base64_encode(input, output);
    return output;
}

inline std::vector<uint8_t> base64_decode(const std::string &input) {
    std::vector<uint8_t> output;
    base64_decode(input, output);
    return output;
}

#endif /* CHROMAPRINT_UTILS_BASE64_HH_ */
