#include <gtest/gtest.h>
#include <dicom-tag.h>
#include "common.h"

TEST(hex, conversions) {
    const char* hex = "00030002"; //element:group
    std::string json_entry = "0002,0003";
    json_entry.append(json_entry.substr(0, 4));
    json_entry.erase(0, 5);
    // after parsing the json our format is element:group
    ASSERT_TRUE(json_entry == hex);
    std::string buffer = hex;
    uint64_t dec = HexToDec(hex);
    ASSERT_TRUE(dec == 196610);
    ASSERT_TRUE(DecToHex(dec,4) == "00030002");
    ASSERT_TRUE(DecToHex(dec,4) == buffer);
    buffer = DecToHex(dec,4);
    buffer.erase(0,4);
    ASSERT_TRUE(buffer == "0002");
}

TEST(hex, mask_extraction) {
    const char* hex = "00030002";
    uint64_t dec = HexToDec(hex);
    ASSERT_TRUE(dec == 196610);
    std::string group = DecToHex(dec,4).erase(0,4);
    ASSERT_TRUE(HexToDec(group) == (dec&GROUP_MASK) );
}