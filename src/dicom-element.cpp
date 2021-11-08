#include <dicom-element.h>
#include <dicom-tag.h>
#include <sstream>

int DicomElement::recursion_counter = 0;

uint32_t DicomElement::CalcLength() {
    require_length = VR == "UT";
    has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
    uint32_t value;

    if (has_reserved) {
        bytes = 12;
        value = *(uint32_t *) (buffer + idx + 8); // 4 bytes
    } else if(tag == dicomTag::Item){
        bytes = 8;
        value = *(uint32_t *) (buffer + idx + 4); // 4 bytes
    } else {
        bytes = 8;
        value = *(uint16_t *) (buffer + idx + 6); // 2 bytes
    }
    return value;
}

uint64_t DicomElement::CalcSize() {
//runs after CalcLength
    if (length == -1) {
        return bytes;
    }
    return length + bytes;
}

std::string DecToHex(uint64_t value, uint8_t bytes) {
    std::stringstream stream;
    stream << std::hex << value;
    std::string ret = std::string(stream.str());
    if(ret.size() & 1){
        ret.insert(0,"0");
    }
    while(ret.size() < bytes*2){
        ret.insert(0,"00");
    }
    return ret;
}
uint32_t HexToDec(std::string hex) {
    uint32_t x;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}