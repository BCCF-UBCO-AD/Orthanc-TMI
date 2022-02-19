#define DEBUG_
#include "../common.h"
#include <gtest/gtest.h>
#include "dicom/dicom-file.h"
#include <filesystem>
#include <functional>
#include <fstream>

namespace fs = std::filesystem;


TEST(dicom, parsing) {
    std::cout << "Unit Test: dicom parsing" << std::endl;
    size_t files_passed = 0;
    size_t files_total = 0;
    std::function<void(const fs::path&)> test = [&](const fs::path &path){
        char msg[1024] = {0};
        files_total++;
        // Allocate buffer
        auto size = fs::file_size(path);
        std::shared_ptr<char[]> buffer(new char[size]);

        // Load file into buffer
        std::ifstream file(path);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());

        // Parse file
        DicomFile dicom(buffer,size);
        ASSERT_TRUE(dicom.IsValid());

        files_passed++;
    };
    //test(GetProjRoot().string() + "/samples/0002.DCM");
    TestWithDicomFiles(test);
    std::cout << files_passed << "/" << files_total << " files passed" << std::endl;
    std::cout << "Test Complete.\n" << std::endl;
}

