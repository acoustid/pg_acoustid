#include <gtest/gtest.h>

#include "base.hpp"

namespace nb = numlib;

TEST(BaseTest, StdVector) {
    EXPECT_EQ(nb::Size(std::vector<float>{}), 0u);
    EXPECT_EQ(nb::Size(std::vector<float>{1, 2, 3}), 3u);

    auto v = nb::Vector(std::vector<float>{1, 2, 3});
    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);

    nb::Resize(v, 4);
    EXPECT_EQ(nb::Size(v), 4u);
}

TEST(BaseTest, StdArray) {
    EXPECT_EQ(nb::Size(std::array<float, 0>{}), 0u);
    EXPECT_EQ(nb::Size(std::array<float, 3>{1, 2, 3}), 3u);

    auto v = nb::Vector(std::array<float, 3>{1, 2, 3});
    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);
}

TEST(BaseTest, StdPair) {
    EXPECT_EQ(nb::Size(std::make_pair(1, 2)), 2u);
}

TEST(BaseTest, StdTuple) {
    EXPECT_EQ(nb::Size(std::make_tuple(0.0, 1.1)), 2u);
}

TEST(BaseTest, ArrayView) {
    float *data = (float *) malloc(sizeof(float) * 3);
    data[0] = 1.0f;
    data[1] = 2.0f;
    data[2] = 3.0f;

    auto v = nb::MakeArray(data, 3);
    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);

    free(data);
}

TEST(BaseTest, ConstArrayView) {
    const float data[] = {1.0f, 2.0f, 3.0f};

    auto v = nb::MakeArray(data, 3);
    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);
}

TEST(BaseTest, OwnedArrayView) {
    float *data = (float *) malloc(sizeof(float) * 3);
    data[0] = 1.0f;
    data[1] = 2.0f;
    data[2] = 3.0f;

    bool free_called = false;

    {
        auto v = nb::MakeArray(data, 3, [&](auto ptr){ free_called = true; free(ptr); });
        EXPECT_EQ(nb::Size(v), 3u);
        EXPECT_EQ(v[0], 1.0f);
        EXPECT_EQ(v[1], 2.0f);
        EXPECT_EQ(v[2], 3.0f);
    }

    EXPECT_TRUE(free_called);
}

TEST(BaseTest, ArrayFromInitializerList) {
    auto v = nb::Array<float>({1, 2, 3});

    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);

    nb::Resize(v, 4);
    EXPECT_EQ(nb::Size(v), 4u);
}

TEST(BaseTest, ArrayFromValue) {
    auto v = nb::Array<float>(3, 1.0);

    EXPECT_EQ(nb::Size(v), 3u);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 1.0f);
    EXPECT_EQ(v[2], 1.0f);

    nb::Resize(v, 4);
    EXPECT_EQ(nb::Size(v), 4u);
}
