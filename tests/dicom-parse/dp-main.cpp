#define DEBUG_
#include <dicom-element.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

extern char* bytebuf_to_hexbuf(std::string bytes);
extern void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
//uint32_t read_tag_code(std::string hex);

int main(int argc, char **argv) {
    std::cout << fs::canonical(".") << std::endl;
    fs::path rel_dicom(fs::canonical("./samples/0002.DCM"));
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