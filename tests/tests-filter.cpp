#include <gtest/gtest.h>
#include <storage-area.h>
#include <dicom-filter.h>
#include <iostream>
#include <fstream>
#include <cstdio>
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
    size_t files_passed = 0;
    size_t files_total = 0;
    auto test = [&](const fs::path &path){
        char msg[1024] = {0};
        files_total++;
        auto size = fs::file_size(path);
        std::unique_ptr<char[]> buffer(new char[size]);

        sprintf(msg,"Loading %s\n", path.c_str());
        DEBUG_LOG(msg);
        std::ifstream file(path, std::ios::binary | std::ios::in);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        DicomFile dicom(buffer.get(), size); // valid
        ASSERT_TRUE(dicom.IsValid()); // pass
        sprintf(msg,"%s file\n",(dicom.IsValid() ? "valid" : "invalid"));
        DEBUG_LOG(msg);

        auto [filtered_buffer,filtered_size] = filter.ApplyFilter(dicom); //memcpy ran til end of file(buffer)
        DicomFile filtered_dicom(filtered_buffer.get(),filtered_size);
        ASSERT_TRUE(filtered_dicom.IsValid()); //first sign the memcpy didn't work
        sprintf(msg,"Filtering: %s\n", (filtered_dicom.IsValid() ? "passed" : "failed"));
        DEBUG_LOG(msg);

        filter.reset();
        files_passed++;
    };
    //test(GetProjRoot().string() + "/samples/0002.DCM");
    TestWithDicomFiles(test);
    std::cout << files_passed << "/" << files_total << " files passed" << std::endl;
    std::cout << "Test Complete." << std::endl;
}