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
    fs::path rel_dicom("./samples/0002.DCM");
    std::ostringstream buf;
    std::ifstream file(rel_dicom);
    ASSERT_TRUE(file.is_open());
    buf << file.rdbuf();
    std::string bufstr(buf.str());
    size_t size = bufstr.size();
    char* hex_buffer = bytebuf_to_hexbuf(bufstr);
    char* buffer = new char[size];
    memcpy(buffer,bufstr.c_str(),bufstr.size());
    char msg_buffer[256] = {0};
    size_t preamble = 128;
    size_t prefix = 4;
    printf("check size\n");
    ASSERT_TRUE(size > preamble + prefix);
    ASSERT_TRUE(std::string_view(buffer+preamble,prefix) == "DICM");
    // move the read head to where the tag data begins
    //buffer = buffer+preamble+prefix;
    printf("read tags\n");
    for(size_t i = preamble+prefix; i < size;){
        DicomElement element(buffer, i, hex_buffer);
        //printf("parse_dicom: bytes: %zu (%zu)\n", element.bytes, element.idx);
        i = element.GetNextIndex();
    }
    delete[] hex_buffer;
    delete[] buffer;
}
