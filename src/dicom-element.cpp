#include <dicom-element.h>
#include <sstream>

std::string DecToHex(uint64_t value) {
    std::stringstream stream;
    stream << std::hex << value;
    std::string ret = std::string(stream.str());
    if(ret.size() & 1){
        ret.insert(0,"0");
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