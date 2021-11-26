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
        std::ifstream file(path);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        DicomFile dicom(buffer.get(), size);
        auto filtered = filter.ApplyFilter(dicom);

        ASSERT_TRUE(fs::exists(path.parent_path().string()));
        fs::path output_path(path.parent_path().string() + "/filtered.dcm");
        std::ofstream output(output_path);
        output.write(std::get<0>(filtered).get(),std::get<1>(filtered));
        output.close();
        ASSERT_TRUE(output.good());
        ASSERT_TRUE(fs::file_size(output_path) <= size);
        fs::remove(output_path);
    };
    TestWithDicomFiles(test);
    std::cout << "Test Complete." << std::endl;
}