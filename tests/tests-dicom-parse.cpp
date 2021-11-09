#include <gtest/gtest.h>
#undef DEBUG_
#include <dicom-element.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

bool parse_file(fs::path dicom);
extern char* hexbuf_to_bytebuf(std::string hex);
extern char* bytebuf_to_hexbuf(std::string bytes);
extern void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
extern uint32_t read_tag_code(std::string hex);

TEST(dicom_parsing, dicom_parse) {
    fs::path exec_path(fs::canonical("/proc/self/exe"));
    fs::path samples;
    fs::path dicom;
    if(exec_path.empty()) {
        samples = fs::path("./samples/");
    } else {
        exec_path = exec_path.parent_path();
        if(exec_path.string().find("build/bin") != std::string::npos){
            exec_path = fs::canonical(exec_path.string() + "/../../");
        }
        samples = fs::path(exec_path.string() + "/samples/");
    }
    // recurse sample directory, parse every file as a dicom
    fs::recursive_directory_iterator recursive_iter(samples);
    for(auto &entry : recursive_iter){
        if(!fs::is_directory(entry.path())){
            auto dicom = entry.path();
            ASSERT_TRUE(parse_file(dicom))
        }
    }
}
bool parse_file(fs::path dicom) {
    // print file path, open ifstream, shove into osstream
    std::cout << dicom;
    std::ostringstream buf;
    std::ifstream file(dicom);
    ASSERT_TRUE(file.is_open());
    buf << file.rdbuf();

    // convert binary sstream into char* buffer
    std::string bufstr(buf.str());
    size_t size = bufstr.size();
    // convert into hex buffer (char*)
    char* hex_buffer = bytebuf_to_hexbuf(bufstr);
    char* buffer = new char[size];
    memcpy(buffer,bufstr.c_str(),bufstr.size());

    // parse buffer
    size_t preamble = 128;
    size_t prefix = 4;
    ASSERT_TRUE(size > preamble + prefix);
    ASSERT_TRUE(std::string_view(buffer+preamble,prefix) == "DICM");
    size_t i = 0;
    // header is OK, parse data elements
    for(i = preamble+prefix; i < size;){
        DicomElement element(buffer, i, hex_buffer);
        //printf("parse_dicom: bytes: %zu (%zu)\n", element.bytes, element.idx);
        i = element.GetNextIndex();
    }
    // did our parser read 100.0% precisely?
    ASSERT_TRUE(i == size);
    delete[] hex_buffer;
    delete[] buffer;
    return i == size;
}
