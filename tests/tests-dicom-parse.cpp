#include <gtest/gtest.h>
#include <dicom-element.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

extern char* hexbuf_to_bytebuf(std::string hex);
extern char* bytebuf_to_hexbuf(std::string bytes);
extern void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
extern uint32_t read_tag_code(std::string hex);

TEST(dicom_parsing, dicom_parse) {
    fs::path rel_dicom("../samples/0002.DCM");
    std::ostringstream buf;
    std::ifstream file(rel_dicom);
    buf << file.rdbuf();
    std::string bufstr(buf.str());
    char* hex_buffer = bytebuf_to_hexbuf(bufstr);
    char* buffer = new char[bufstr.size()];
    memcpy(buffer,bufstr.c_str(),bufstr.size());
    parse_dicom(hex_buffer, buffer, bufstr.size());
    delete[] hex_buffer;
    delete[] buffer;
}
