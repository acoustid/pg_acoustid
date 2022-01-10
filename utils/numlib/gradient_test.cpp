#include <gtest/gtest.h>

#include "gradient.hpp"

namespace nb = numlib;

TEST(GradientTest, Gradient) {
    auto a = nb::Vector({1.0, 2.0, 4.0, 7.0, 11.0, 16.0});

    EXPECT_EQ(nb::Gradient(a), nb::Vector({1. , 1.5, 2.5, 3.5, 4.5, 5.}));
}
