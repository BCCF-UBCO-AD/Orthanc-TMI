#include <gtest/gtest.h>
#define DEBUG_
#include <dicom-file.h>
#include <filesystem>
#include <functional>
#include <fstream>
#include "common.h"

namespace fs = std::filesystem;


TEST(dicom_parsing, dicom_parse) {
    std::function<void(const fs::path&)> test = [](const fs::path &path){
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
    };
    TestWithDicomFiles(test);
    std::cout << "Test Complete." << std::endl;
}

