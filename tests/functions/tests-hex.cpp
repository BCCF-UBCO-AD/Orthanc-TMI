#include <gtest/gtest.h>
#include "dicom/dicom-tag.h"
#include "../common.h"
std::string group = "0002";
std::string key = "0002,0003";
std::string hex = "00030002";
uint64_t dec = 196610;

TEST(functions, hex_conversions) {
    ASSERT_EQ(KeyToHex(key),hex);
    ASSERT_EQ(HexToKey(hex),key);
    ASSERT_EQ(HexToDec(hex),dec);
    ASSERT_EQ(DecToHex(dec,4),hex);
}

TEST(functions, hex_mask_extraction) {
    ASSERT_EQ(dec&GROUP_MASK, HexToDec(group));
    ASSERT_EQ(dec&GROUP_MASK, HexToDec(KeyToHex(key))&GROUP_MASK);
}