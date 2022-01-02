#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "packed_uint5_array.hh"

using testing::ElementsAre;

TEST(PackedUInt5ArrayTest, GetPackedSize) {
    EXPECT_EQ(get_packed_uint5_array_size(1), 1);
    EXPECT_EQ(get_packed_uint5_array_size(2), 2);
    EXPECT_EQ(get_packed_uint5_array_size(3), 2);
    EXPECT_EQ(get_packed_uint5_array_size(4), 3);
    EXPECT_EQ(get_packed_uint5_array_size(5), 4);
}

TEST(PackedUInt5ArrayTest, Pack) {
    EXPECT_THAT(pack_uint5_array({1}), ElementsAre(0x1));
    EXPECT_THAT(pack_uint5_array({1, 2}), ElementsAre(0x41, 0x00));
    EXPECT_THAT(pack_uint5_array({1, 2, 3}), ElementsAre(0x41, 0x0C));
    EXPECT_THAT(pack_uint5_array({1, 2, 3, 4}), ElementsAre(0x41, 0x0C, 0x02));
    EXPECT_THAT(pack_uint5_array({1, 2, 3, 4, 5}), ElementsAre(0x41, 0x0C, 0x52, 0x00));
}

TEST(PackedUInt5ArrayTest, GetUnpackedSize) {
    EXPECT_EQ(get_unpacked_uint5_array_size(1), 1);
    EXPECT_EQ(get_unpacked_uint5_array_size(2), 3);
    EXPECT_EQ(get_unpacked_uint5_array_size(3), 4);
    EXPECT_EQ(get_unpacked_uint5_array_size(4), 6);
}

TEST(PackedUInt5ArrayTest, Unpack) {
    EXPECT_THAT(unpack_uint5_array({0x01}), ElementsAre(1));
    EXPECT_THAT(unpack_uint5_array({0x41, 0x00}), ElementsAre(1, 2, 0));
    EXPECT_THAT(unpack_uint5_array({0x41, 0x0C}), ElementsAre(1, 2, 3));
    EXPECT_THAT(unpack_uint5_array({0x41, 0x0C, 0x02}), ElementsAre(1, 2, 3, 4));
    EXPECT_THAT(unpack_uint5_array({0x41, 0x0C, 0x52, 0x00}), ElementsAre(1, 2, 3, 4, 5, 0));
}
