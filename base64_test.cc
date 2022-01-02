#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "base64.hh"

using testing::ElementsAre;

TEST(Base64Test, Validate) {
    EXPECT_TRUE(base64_validate(""));
    EXPECT_FALSE(base64_validate("Y"));
    EXPECT_TRUE(base64_validate("YQ"));
    EXPECT_TRUE(base64_validate("YWI"));
    EXPECT_TRUE(base64_validate("YWJj"));
    EXPECT_FALSE(base64_validate("YWJjZ"));
    EXPECT_TRUE(base64_validate("YWJjZA"));
    EXPECT_TRUE(base64_validate("YWJjZGU"));
    EXPECT_FALSE(base64_validate("!x"));
}

TEST(Base64Test, Decode) {
    EXPECT_THAT(base64_decode(""), ElementsAre());
    EXPECT_THAT(base64_decode("Y"), ElementsAre()); // invalid
    EXPECT_THAT(base64_decode("YQ"), ElementsAre('a'));
    EXPECT_THAT(base64_decode("YWI"), ElementsAre('a', 'b'));
    EXPECT_THAT(base64_decode("YWJj"), ElementsAre('a', 'b', 'c'));
    EXPECT_THAT(base64_decode("YWJjZ"), ElementsAre('a', 'b', 'c')); // invalid
    EXPECT_THAT(base64_decode("YWJjZA"), ElementsAre('a', 'b', 'c', 'd'));
    EXPECT_THAT(base64_decode("YWJjZGU"), ElementsAre('a', 'b', 'c', 'd', 'e'));
}

TEST(Base64Test, Encode) {
    EXPECT_EQ("", base64_encode({}));
    EXPECT_EQ("AA", base64_encode({0}));
    EXPECT_EQ("YQ", base64_encode({'a'}));
    EXPECT_EQ("YWI", base64_encode({'a', 'b'}));
    EXPECT_EQ("YWJj", base64_encode({'a', 'b', 'c'}));
    EXPECT_EQ("YWJjZA", base64_encode({'a', 'b', 'c', 'd'}));
    EXPECT_EQ("YWJjZGU", base64_encode({'a', 'b', 'c', 'd', 'e'}));
}
