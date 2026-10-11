#include "../common.h"
#include <gtest/gtest.h>
#include <dicom-tag.h>

std::string group = "0002";
std::string tag_key = "0002,0003";
std::string hex = "00030002";
uint64_t tag = 196610;

TEST(functions, hex_conversions) {
    std::cout << "Unit Test: functions hex_conversions" << std::endl;
    ASSERT_EQ(KeyToHex(tag_key),hex);
    ASSERT_EQ(HexToKey(hex),tag_key);
    ASSERT_EQ(HexToDec(hex),tag);
    ASSERT_EQ(DecToHex(tag),hex);
    std::cout << "Test Complete.\n" << std::endl;
}

TEST(functions, hex_mask_extraction) {
    std::cout << "Unit Test: functions hex_mask_extraction" << std::endl;
    ASSERT_EQ(tag&GROUP_MASK, HexToDec(group));
    ASSERT_EQ(tag&GROUP_MASK, HexToDec(KeyToHex(tag_key))&GROUP_MASK);
    std::cout << "Test Complete.\n" << std::endl;
}