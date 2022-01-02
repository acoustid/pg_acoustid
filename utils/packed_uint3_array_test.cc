#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "packed_uint3_array.hh"

using testing::ElementsAre;

TEST(PackedUInt3ArrayTest, GetPackedSize) {
    EXPECT_EQ(get_packed_uint3_array_size(1), 1);
    EXPECT_EQ(get_packed_uint3_array_size(2), 1);
    EXPECT_EQ(get_packed_uint3_array_size(3), 2);
    EXPECT_EQ(get_packed_uint3_array_size(4), 2);
    EXPECT_EQ(get_packed_uint3_array_size(5), 2);
}

TEST(PackedUInt3ArrayTest, Pack) {
    EXPECT_THAT(pack_uint3_array({1}), ElementsAre(0x1));
    EXPECT_THAT(pack_uint3_array({1, 2}), ElementsAre(0x11));
    EXPECT_THAT(pack_uint3_array({1, 2, 3}), ElementsAre(0xD1, 0x00));
    EXPECT_THAT(pack_uint3_array({1, 2, 3, 4}), ElementsAre(0xD1, 0x08));
    EXPECT_THAT(pack_uint3_array({1, 2, 3, 4, 5}), ElementsAre(0xD1, 0x58));
}

TEST(PackedUInt3ArrayTest, GetUnpackedSize) {
    EXPECT_EQ(get_unpacked_uint3_array_size(1), 2);
    EXPECT_EQ(get_unpacked_uint3_array_size(2), 5);
}

TEST(PackedUInt3ArrayTest, Unpack) {
    EXPECT_THAT(unpack_uint3_array({0x01}), ElementsAre(1, 0));
    EXPECT_THAT(unpack_uint3_array({0x11}), ElementsAre(1, 2));
    EXPECT_THAT(unpack_uint3_array({0xD1, 0x00}), ElementsAre(1, 2, 3, 0, 0));
    EXPECT_THAT(unpack_uint3_array({0xD1, 0x08}), ElementsAre(1, 2, 3, 4, 0));
    EXPECT_THAT(unpack_uint3_array({0xD1, 0x58}), ElementsAre(1, 2, 3, 4, 5));
}
