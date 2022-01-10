#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "minmax.hpp"

namespace nb = numlib;

TEST(MinMaxTest, ArgRelMin) {
    EXPECT_THAT(
        nb::ArgRelMin(nb::Array({2, 1, 2, 3, 2, 0, 1, 0})),
        testing::Pointwise(testing::Eq(), nb::Array({1u, 5u}))
    );
}

TEST(MinMaxTest, ArgRelMax) {
    EXPECT_THAT(
        nb::ArgRelMax(nb::Array({2, 1, 2, 3, 2, 0, 1, 0})),
        testing::Pointwise(testing::Eq(), nb::Array({3u, 6u}))
    );
}
