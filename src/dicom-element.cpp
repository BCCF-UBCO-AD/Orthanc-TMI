#include <dicom-element.h>
#include <sstream>

std::string DecToHex(uint16_t value) {
    std::stringstream stream;
    stream << std::hex << value;
    return std::string(stream.str());
}
uint32_t HexToDec(std::string hex) {
    uint32_t x;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}