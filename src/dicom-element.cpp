#include <dicom-element.h>
#include <dicom-tag.h>
#include <sstream>

uint32_t DicomElement::CalcLength() {
    // length width, and location must be deduced
    // Dicom Element Structure: https://www.leadtools.com/help/sdk/v21/dicom/api/overview-data-element-structure.html#!
    require_length = VR == "UT";
    has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
    uint32_t value;

    if (has_reserved) {
        // structure includes reserved bytes after the VR
        bytes = 12;
        value = *(uint32_t *) (buffer + idx + 8); // 4 bytes
    } else if(tag == dicomTag::Item){
        // structure has an implicit VR
        bytes = 8;
        value = *(uint32_t *) (buffer + idx + 4); // 4 bytes
    } else {
        // structure with an explicit VR but no reserved bytes
        bytes = 8;
        value = *(uint16_t *) (buffer + idx + 6); // 2 bytes
    }
    return value;
}

uint64_t DicomElement::CalcSize() {
    //runs after CalcLength
    if (length == -1) {
        // Dicom Element is only as big as the header
        return bytes;
    }
    // Dicom Element has an explicit value length
    return length + bytes;
}

std::string DecToHex(uint64_t value, uint8_t bytes) {
    std::stringstream stream;
    stream << std::hex << value;
    std::string ret = std::string(stream.str());
    if(ret.size() & 1){
        ret.insert(0,"0");
    }
    // add 00 padding to ensure we reach the correct size
    while(ret.size() < bytes*2){
        ret.insert(0,"00");
    }
    return ret;
}
uint64_t HexToDec(std::string hex) {
    uint64_t x;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}