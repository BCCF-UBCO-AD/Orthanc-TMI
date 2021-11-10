#define DEBUG_
#include <dicom-element.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cassert>

namespace fs = std::filesystem;

extern char* bytebuf_to_hexbuf(std::string bytes);
void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
//uint32_t read_tag_code(std::string hex);

int main(int argc, char **argv) {
    std::cout << fs::canonical(".") << std::endl;
    fs::path rel_dicom(fs::canonical("../../samples/0002.DCM"));
    std::ostringstream buf;
    std::ifstream file(rel_dicom);
    std::cout << rel_dicom << std::endl;
    buf << file.rdbuf();
    std::string bufstr(buf.str());
    char* hex_buffer = bytebuf_to_hexbuf(bufstr);
    char* buffer = new char[bufstr.size()];
    memcpy(buffer,bufstr.c_str(),bufstr.size());
    parse_dicom(hex_buffer, buffer, bufstr.size());
    delete[] hex_buffer;
    delete[] buffer;
    return 0;
}

void parse_dicom(const char* hex_buffer, const char* buffer, size_t size) {
    printf("parsing dicom, size: %zu Bytes\n",size);
    char msg_buffer[256] = {0};
    size_t preamble = 128;
    size_t prefix = 4;
    printf("check size\n");
    assert(size > preamble + prefix);
    printf("check DICM\n");
    std::cout << std::string(std::string_view(buffer+preamble,prefix)) << std::endl;
    assert(std::string_view(buffer+preamble,prefix) == "DICM");
    // move the read head to where the tag data begins
    //buffer = buffer+preamble+prefix;
    printf("read tags\n");
    for(size_t i = preamble+prefix; i < size;){
        DicomElement element(buffer, i, hex_buffer);
        //printf("parse_dicom: bytes: %zu (%zu)\n", element.bytes, element.idx);
        i = element.GetNextIndex();
    }
}