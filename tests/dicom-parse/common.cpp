#define DEBUG_
#include <dicom-element.h>
#include <sstream>
#include <iostream>
#include <cassert>
#include <vector>

void parse_dicom(const char* hex_buffer, const char* buffer, size_t size) {
    printf("parsing dicom, size: %zu Bytes\n",size);
    char msg_buffer[256] = {0};
    size_t preamble = 128;
    size_t prefix = 4;
    printf("check size\n");
    assert(size > preamble + prefix);
    std::cout << std::string(std::string_view(buffer+preamble,prefix)) << std::endl;
    printf("check DICM\n");
    assert(std::string_view(buffer+preamble,prefix) == "DICM");
    // move the read head to where the tag data begins
    //buffer = buffer+preamble+prefix;
    printf("read tags\n");
    for(size_t i = preamble+prefix; i < size;){
        DicomElement element(buffer, i);
        i = element.GetNextIndex();
    }
}

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