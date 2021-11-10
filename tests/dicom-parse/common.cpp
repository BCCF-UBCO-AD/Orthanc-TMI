#include <dicom-element.h>
#include <sstream>
#include <iostream>
#include <vector>

char* hexbuf_to_bytebuf(std::string hex){
    std::stringstream ss;
    //std::string hex = dicom_file_excerpt;
    std::vector<char> hexCh;
    unsigned int temp;
    int integer_value;
    int offset = 0;
    while (offset < hex.length()) {
        ss.clear();
        ss << std::hex << hex.substr(offset, 2);
        ss >> temp;
        hexCh.push_back(static_cast<char>(temp));
        offset += 2;
    }
    char* buffer = new char[hexCh.size()+1];
    offset = 0;
    for(auto c : hexCh){
        buffer[offset++] = c;
        // printf("%c",c);
    }
    std::cout << "size: " << offset-1 << " bytes" << std::endl;
    return buffer;
}

char* bytebuf_to_hexbuf(std::string bytes){
    std::stringstream hexbuf;
    for(char &c : bytes){
        hexbuf << DecToHex(c);
    }
    std::string hex(hexbuf.str());
    char* buffer = new char[hex.size()];
    memcpy(buffer,hex.c_str(),hex.size());
    return buffer;
}

uint32_t shift(uint8_t value, uint8_t shift_amount){
    return value << shift_amount;
}

uint32_t read_tag_code(std::string hex){
    uint8_t byte[4];
    byte[0] = HexToDec(std::string(std::string_view(hex.c_str(),2)));
    byte[1] = HexToDec(std::string(std::string_view(hex.c_str()+2,2)));
    byte[2] = HexToDec(std::string(std::string_view(hex.c_str()+4,2)));
    byte[3] = HexToDec(std::string(std::string_view(hex.c_str()+6,2)));
    return shift(byte[0],8) | shift(byte[1],0) | shift(byte[2],24) | shift(byte[3],16);
}