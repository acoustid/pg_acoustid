#include <gtest/gtest.h>

#include "convolve.hpp"

namespace nb = numlib;

TEST(ConvolveTest, ConvolveFull) {
    auto a = nb::Vector({1.0, 2.0, 3.0});
    auto v = nb::Vector({0.0, 1.0, 0.5});

    EXPECT_EQ(nb::Convolve(a, v, nb::ConvolveMode::FULL), nb::Vector({0.0, 1.0, 2.5, 4.0, 1.5}));
}

TEST(ConvolveTest, ConvolveSame) {
    auto a = nb::Vector({1.0, 2.0, 3.0});
    auto v = nb::Vector({0.0, 1.0, 0.5});

    EXPECT_EQ(nb::Convolve(a, v, nb::ConvolveMode::SAME), nb::Vector({1.0, 2.5, 4.0}));
}

TEST(ConvolveTest, ConvolveValid) {
    auto a = nb::Vector({1.0, 2.0, 3.0});
    auto v = nb::Vector({0.0, 1.0, 0.5});

    EXPECT_EQ(nb::Convolve(a, v, nb::ConvolveMode::VALID), nb::Vector({2.5}));
}
