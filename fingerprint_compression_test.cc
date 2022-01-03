#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "base64.hh"
#include "fingerprint_compression.h"

TEST(CompressedFingerprintTest, GetVersion) {
    std::vector<uint8_t> fp;
    fp = base64_decode("AAAAA4qkJVGqKIH_AgUA");
    EXPECT_EQ(compressed_fingerprint_get_version(fp.data(), fp.size()), 0);
    fp = base64_decode("AQAAA4qkJVGqKIH_AgUA");
    EXPECT_EQ(compressed_fingerprint_get_version(fp.data(), fp.size()), 1);
    fp = base64_decode("");
    EXPECT_EQ(compressed_fingerprint_get_version(fp.data(), fp.size()), -1);
    fp = base64_decode("A");
    EXPECT_EQ(compressed_fingerprint_get_version(fp.data(), fp.size()), -1);
}

TEST(CompressedFingerprintTest, GetNumTerms) {
    std::vector<uint8_t> fp;
    fp = base64_decode("AAAAA4qkJVGqKIH_AgUA");
    EXPECT_EQ(compressed_fingerprint_get_num_terms(fp.data(), fp.size()), 3u);
    fp = base64_decode("AQAAA4qkJVGqKIH_AgUA");
    EXPECT_EQ(compressed_fingerprint_get_num_terms(fp.data(), fp.size()), 3u);
    fp = base64_decode("");
    EXPECT_EQ(compressed_fingerprint_get_num_terms(fp.data(), fp.size()), 0u);
    fp = base64_decode("A");
    EXPECT_EQ(compressed_fingerprint_get_num_terms(fp.data(), fp.size()), 0u);
}

TEST(CompressedFingerprintTest, GetTerms) {
    std::vector<uint8_t> fp;
    std::vector<uint32_t> terms;
    fp = base64_decode("AAAAA4qkJVGqKIH_AgUA");
    terms.resize(compressed_fingerprint_get_num_terms(fp.data(), fp.size()));
    EXPECT_EQ(compressed_fingerprint_get_terms(fp.data(), fp.size(), terms.data(), terms.size()), 0);
    EXPECT_THAT(terms, testing::ElementsAre(994277718, 440498550, 440498550));
}

TEST(CompressedFingerprintTest, GetTermsNotEnoughDataForHeader) {
    std::vector<uint8_t> fp;
    std::vector<uint32_t> terms;
    fp = base64_decode("");
    terms.resize(compressed_fingerprint_get_num_terms(fp.data(), fp.size()));
    EXPECT_EQ(compressed_fingerprint_get_terms(fp.data(), fp.size(), terms.data(), terms.size()), -1);
}

TEST(CompressedFingerprintTest, GetTermsNotEnoughDataForNormalBits) {
    std::vector<uint8_t> fp;
    std::vector<uint32_t> terms;
    fp = base64_decode("AAAAA4qkJVGqKI");
    terms.resize(compressed_fingerprint_get_num_terms(fp.data(), fp.size()));
    EXPECT_EQ(compressed_fingerprint_get_terms(fp.data(), fp.size(), terms.data(), terms.size()), -1);
}

TEST(CompressedFingerprintTest, GetTermsNotEnoughDataForExceptionalBits) {
    std::vector<uint8_t> fp;
    std::vector<uint32_t> terms;
    fp = base64_decode("AAAAA4qkJVGqKIH_AgU");
    terms.resize(compressed_fingerprint_get_num_terms(fp.data(), fp.size()));
    EXPECT_EQ(compressed_fingerprint_get_terms(fp.data(), fp.size(), terms.data(), terms.size()), -1);
}

TEST(CompressedFingerprintTest, GetTermsTrailingData) {
    std::vector<uint8_t> fp;
    std::vector<uint32_t> terms;
    fp = base64_decode("AAAAA4qkJVGqKIH_AgUAAA");
    terms.resize(compressed_fingerprint_get_num_terms(fp.data(), fp.size()));
    EXPECT_EQ(compressed_fingerprint_get_terms(fp.data(), fp.size(), terms.data(), terms.size()), -1);
}
