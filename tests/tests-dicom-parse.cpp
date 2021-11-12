#include <gtest/gtest.h>
#undef DEBUG_
#include <dicom-element.h>
#include <filesystem>
#include <functional>
#include <fstream>

namespace fs = std::filesystem;

// prototype for the TEST to use
bool parse_file(fs::path dicom, std::function<void(bool)> gassert);
TEST(dicom_parsing, dicom_parse) {
    // linux only line to acquire the path of the executable
    fs::path exec_path(fs::canonical("/proc/self/exe"));
    fs::path samples;
    if(exec_path.empty()) {
        // if we're on windows try ./samples as the relative path
        samples = fs::path("./samples/"); // probably won't work
    } else {
        // if we're on linux, then get the directory the executable is stored in
        exec_path = exec_path.parent_path();
        // if that directory is the build/bin, then our relative path is ../../samples
        if(exec_path.string().find("build/bin") != std::string::npos){
            exec_path = fs::canonical(exec_path.string() + "/../../");
        }
        samples = fs::path(exec_path.string() + "/samples/");
    }
    // ASSERT_TRUE won't compile in regular functions, so we make a lambda to pass it to other functions
    std::function<void(bool)> gassert = [&](bool v){ASSERT_TRUE(v);};
    // recurse sample directory, parse every file as a dicom
    fs::recursive_directory_iterator recursive_iter(samples);
    for(auto &entry : recursive_iter){
        auto &path = entry.path();
        if(!fs::is_directory(path)){
            if(path.extension().string() == ".DCM"){
                const fs::path &dicom = path;
                std::cout << "Parse " << dicom << std::endl;
                // The test passes if parse_file returns true
                ASSERT_TRUE(parse_file(dicom, gassert));
            }
        }
    }
}

// prototypes for parse_file to use
extern char* bytebuf_to_hexbuf(std::string bytes);
extern void parse_dicom(const char* hex_buffer, const char* buffer, size_t size);
bool parse_file(fs::path dicom, std::function<void(bool)> gassert) {
    // print file path, open ifstream, shove into osstream
    std::ostringstream buf;
    std::ifstream file(dicom);
    gassert(file.is_open());
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
    gassert(size > preamble + prefix);
    gassert(std::string_view(buffer+preamble,prefix) == "DICM");
    size_t i = 0;
    // header is OK, parse data elements
    for(i = preamble+prefix; i < size;){
        DicomElement element(buffer, i, hex_buffer);
        //printf("parse_dicom: bytes: %zu (%zu)\n", element.bytes, element.idx);
        i = element.GetNextIndex();
    }
    // did our parser read 100.0% precisely?
    gassert(i == size);
    delete[] hex_buffer;
    delete[] buffer;
    return i == size;
}