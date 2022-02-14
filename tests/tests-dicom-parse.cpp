#define DEBUG_
#include "common.h"
#include <gtest/gtest.h>
#include <dicom-file.h>
#include <filesystem>
#include <functional>
#include <fstream>

namespace fs = std::filesystem;


TEST(dicom_parsing, dicom_parse) {
    std::function<void(const fs::path&)> test = [](const fs::path &path){
        std::cout << "Parse " << path << std::endl;
        auto size = fs::file_size(path);
        std::unique_ptr<char[]> buffer(new char[size]);
        std::ifstream file(path);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        DicomFile dicom(buffer.get(),size);
        ASSERT_TRUE(dicom.IsValid());
    };
    test(GetProjRoot().string() + "/samples/0002.DCM");
    //TestWithDicomFiles(test);
    std::cout << "Test Complete." << std::endl;
}

