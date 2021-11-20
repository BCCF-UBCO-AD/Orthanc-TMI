#define DEBUG_
#include <dicom-element.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cassert>

namespace fs = std::filesystem;

extern std::unique_ptr<char[]> bytebuf_to_hexbuf(std::string bytes);
void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
//uint32_t read_tag_code(std::string hex);

int main(int argc, char **argv) {
    std::cout << fs::canonical(".") << std::endl;

    fs::path path(fs::canonical("../../samples/0002.DCM"));
    std::cout << path << std::endl;

    std::ifstream file(path);
    size_t size = fs::file_size(path);
    auto buffer = std::unique_ptr<char[]>(new char[size]);
    file.read(buffer.get(),size);
    file.close();
    auto hex_buffer = bytebuf_to_hexbuf(buffer.get());
    parse_dicom(hex_buffer.get(), buffer.get(), size);
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