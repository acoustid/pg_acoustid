#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "window.hpp"

namespace nb = numlib;

TEST(WindowTest, GaussianSymetric) {
    EXPECT_THAT(
        nb::GaussianWindow(8, 1.8, true),
        testing::Pointwise(testing::FloatNear(1.0 / 1024), nb::Array({
            0.15100654, 0.38117139, 0.70664828, 0.96215449,
            0.96215449, 0.70664828, 0.38117139, 0.15100654,
        }))
    );
    EXPECT_THAT(
        nb::GaussianWindow(9, 1.8, true),
        testing::Pointwise(testing::FloatNear(1.0 / 1024), nb::Array({
            0.08465798862252998, 0.24935220877729622, 0.5394075072376266, 0.8569968914352789,
            1.0,
            0.8569968914352789, 0.5394075072376266, 0.24935220877729622, 0.08465798862252998
        }))
    );
}

TEST(WindowTest, GaussianPeriodic) {
    EXPECT_THAT(
        nb::GaussianWindow(8, 1.8, false),
        testing::Pointwise(testing::FloatNear(1.0 / 1024), nb::Array({
            0.08465799, 0.24935221, 0.53940751, 0.85699689,
            1.00000000, 0.85699689, 0.53940751, 0.24935221,
        }))
    );
    EXPECT_THAT(
        nb::GaussianWindow(9, 1.8, false),
        testing::Pointwise(testing::FloatNear(1.0 / 1024), nb::Array({
            0.04393693362340742, 0.15100654499550906, 0.3811713860529925, 0.7066482778577162, 0.962154491713458,
            0.962154491713458, 0.7066482778577162, 0.3811713860529925, 0.15100654499550906
        }))
    );
}
