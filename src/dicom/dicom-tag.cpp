#include <dicom-tag.h>
#include <sstream>

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