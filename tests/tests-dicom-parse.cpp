#include <gtest/gtest.h>
#define DEBUG_
#include <dicom-file.h>
#include <filesystem>
#include <functional>
#include <fstream>

namespace fs = std::filesystem;

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

    // recurse sample directory, parse every file as a dicom
    fs::recursive_directory_iterator recursive_iter(samples);
    for(auto &entry : recursive_iter){
        auto &path = entry.path();
        if(!fs::is_directory(path)){
            if(path.extension().string() == ".DCM"){
                std::cout << "Parse " << path << std::endl;
                auto size = fs::file_size(path);
                char* buffer = new char[size];
                std::ifstream file(path);
                ASSERT_TRUE(file.is_open());
                file.read(buffer,size);
                file.close();
                DicomFile dicom(buffer,size);
                ASSERT_TRUE(dicom.IsValid());
                delete[] buffer;
            }
        }
    }
}

