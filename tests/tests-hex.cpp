#include <gtest/gtest.h>
#include <dicom-tag.h>
#include "common.h"
std::string group = "0002";
std::string key = "0002,0003";
std::string hex = "00030002";
uint64_t dec = 196610;

TEST(hex, conversions) {
    ASSERT_EQ(KeyToHex(key),hex);
    ASSERT_EQ(HexToKey(hex),key);
    ASSERT_EQ(HexToDec(hex),dec);
    ASSERT_EQ(DecToHex(dec,4),hex);
}

TEST(hex, mask_extraction) {
    ASSERT_EQ(HexToDec(group),dec&GROUP_MASK);
}