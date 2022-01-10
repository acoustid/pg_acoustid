#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include "align_fingerprints.h"
#include "numlib/array.hpp"

namespace nb = numlib;

static std::string read_file(const std::string& filename) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static void parse_fingerprint_json(const std::string &json, std::vector<uint32_t> &terms) {
    auto pos = json.find("\"fingerprint\": [");
    if (pos == std::string::npos) {
        ASSERT_EQ(json, "");
        throw std::runtime_error("Failed to find fingerprint");
    }
    pos += 16;
    auto end = json.find("]", pos);
    if (end == std::string::npos) {
        throw std::runtime_error("Failed to find fingerprint end");
    }
    auto terms_str = json.substr(pos, end - pos);
    terms.clear();
    while (!terms_str.empty()) {
        auto comma = terms_str.find(",");
        if (comma == std::string::npos) {
            terms.push_back(std::stoul(terms_str));
            break;
        }
        terms.push_back(std::stoul(terms_str.substr(0, comma)));
        terms_str = terms_str.substr(comma + 1);
    }
}

TEST(AlignFingerprintsTest, KeepYourTipsUpBackgroundMusic) {
    std::vector<uint32_t> fp1, fp2;
    int offsets[10], scores[10];

    // Full music track vs. TV show in which part of the track is used
    // https://www.youtube.com/watch?v=wyrAUJCkDDc
    // https://www.redbullsoundsupply.com/tracks/titan

    parse_fingerprint_json(read_file("testdata/keep_your_tips_up.json"), fp1);
    parse_fingerprint_json(read_file("testdata/titan.json"), fp2);

    auto num_peaks = AlignFingerprints(fp1.data(), fp1.size(), fp2.data(), fp2.size(), offsets, scores, 10);
    EXPECT_THAT(nb::MakeArray(offsets, num_peaks), testing::ElementsAre(-243, -71, -329, -588));
}

TEST(AlignFingerprintsTest, RoadVineSnippet) {
    std::vector<uint32_t> fp1, fp2;
    int offsets[10], scores[10];

    parse_fingerprint_json(read_file("testdata/roar.json"), fp1);
    parse_fingerprint_json(read_file("testdata/roar_vine.json"), fp2);

    auto num_peaks = AlignFingerprints(fp1.data(), fp1.size(), fp2.data(), fp2.size(), offsets, scores, 10);
    EXPECT_THAT(nb::MakeArray(offsets, num_peaks), testing::ElementsAre(528));
}

TEST(AlignFingerprintsTest, TheWorldIsAllOneDifferentVocals) {
    std::vector<uint32_t> fp1, fp2;
    int offsets[10], scores[10];

    parse_fingerprint_json(read_file("testdata/the_world_is_all_one_02.json"), fp1);
    parse_fingerprint_json(read_file("testdata/the_world_is_all_one_03.json"), fp2);

    auto num_peaks = AlignFingerprints(fp1.data(), fp1.size(), fp2.data(), fp2.size(), offsets, scores, 10);
    EXPECT_THAT(nb::MakeArray(offsets, num_peaks), testing::ElementsAre(0));
}

TEST(AlignFingerprintsTest, DifferentTracks1) {
    std::vector<uint32_t> fp1, fp2;
    int offsets[10], scores[10];

    parse_fingerprint_json(read_file("testdata/the_world_is_all_one_02.json"), fp1);
    parse_fingerprint_json(read_file("testdata/titan.json"), fp2);

    auto num_peaks = AlignFingerprints(fp1.data(), fp1.size(), fp2.data(), fp2.size(), offsets, scores, 10);
    EXPECT_THAT(nb::MakeArray(offsets, num_peaks), testing::ElementsAre());
}

TEST(AlignFingerprintsTest, DifferentTracks2) {
    std::vector<uint32_t> fp1, fp2;
    int offsets[10], scores[10];

    parse_fingerprint_json(read_file("testdata/roar.json"), fp1);
    parse_fingerprint_json(read_file("testdata/titan.json"), fp2);

    auto num_peaks = AlignFingerprints(fp1.data(), fp1.size(), fp2.data(), fp2.size(), offsets, scores, 10);
    EXPECT_THAT(nb::MakeArray(offsets, num_peaks), testing::ElementsAre());
}
