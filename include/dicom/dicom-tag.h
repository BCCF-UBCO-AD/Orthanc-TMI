#pragma once
#include <string>
#define GROUP_MASK ((1<<16)-1)
using tag_code = uint64_t;

// convert decimal to hex, and pad the string to size `2*bytes`
extern std::string DecToHex(uint64_t value, uint8_t bytes = 1);

// convert up to 8 bytes of hex to decimal
extern uint64_t HexToDec(std::string hex);

// convert hex tag to tag key (a0a0b0b0 => b0b0,a0a0)
extern std::string HexToKey(std::string hex);

// convert tag key to hex tag (b0b0,a0a0 => a0a0b0b0)
extern std::string KeyToHex(std::string key);

// Dicom Tags/VR/Name found here https://www.dicomlibrary.com/dicom/dicom-tags/
enum DicomTag{
    Item = 0xe000fffe,
    ItemDelItem = 0xe00dfffe,
    SeqDelItem = 0xe0ddfffe
};