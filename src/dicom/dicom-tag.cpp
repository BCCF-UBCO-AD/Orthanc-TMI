#include <dicom-tag.h>
#include <sstream>

std::string DecToHex(tag_uint64_t tag, uint8_t bytes) {
    std::stringstream stream;
    stream << std::hex << tag;
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
tag_uint64_t HexToDec(std::string hex) {
    tag_uint64_t x;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}

std::string HexToKey(std::string hex) {
    hex.append(",");
    hex.append(hex.substr(0,4));
    hex.erase(0,4);
    return hex;
}

std::string KeyToHex(std::string hex) {
    if(hex.length() == 9){
        hex.append(hex.substr(0, 4));
        hex.erase(0, 5);
        return hex;
    }
    return "";
}