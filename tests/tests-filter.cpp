#include <gtest/gtest.h>
#include <storage-area.h>
#include <dicom-filter.h>
#include <iostream>
#include <fstream>
#include "common.h"

extern OrthancPluginErrorCode WriteDicomFile(DicomFile dicom, const char *uuid);

TEST(filtering, store_filtered) {
    fs::path config_path(GetProjRoot().string() + "/docker/orthanc/orthanc.json");
    nlm::json config;
    std::cout << "Loading config.." << std::endl;
    if(fs::exists(config_path)) {
        std::ifstream file(config_path);
        ASSERT_TRUE(file.is_open());
        file >> config;
        ASSERT_TRUE(file.good());
        file.close();
    } else {
        ASSERT_TRUE(false);
    }
    std::cout << "Configuring DicomFilter.." << std::endl;
    DicomFilter filter = DicomFilter::ParseConfig(config);
    std::cout << " configured" << std::endl;
    auto test = [&](const fs::path &path){
        auto size = fs::file_size(path);
        std::unique_ptr<char[]> buffer(new char[size]);
        std::cout << "Loading " << path << std::endl;
        std::ifstream file(path, std::ios::binary | std::ios::in);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        std::cout << "Parsing.." << std::endl;
        DicomFile dicom(buffer.get(), size); // valid
        ASSERT_TRUE(dicom.IsValid()); // pass
        std::cout << "Loaded " << (dicom.IsValid() ? "valid" : "invalid") << " dicom file." << std::endl;
        std::cout << "Filtering.." << std::endl;
        auto [filtered_buffer,filtered_size] = filter.ApplyFilter(dicom); //memcpy ran til end of file(buffer)
        DicomFile filtered_dicom(filtered_buffer.get(),filtered_size);
        ASSERT_TRUE(filtered_dicom.IsValid()); //first sign the memcpy didn't work
        std::cout << "Filtered data: " << (filtered_dicom.IsValid() ? "valid" : "invalid") << std::endl;
        filter.reset();
        std::cout << "  FILE PASSED\n" << std::endl;
    };
    //test(GetProjRoot().string() + "/samples/0002.DCM");
    TestWithDicomFiles(test);
    std::cout << "Test Complete." << std::endl;
}