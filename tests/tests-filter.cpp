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
        std::cout << "Filtering " << path << std::endl;
        std::ifstream file(path);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        std::cout << "DicomFile construction" << std::endl;
        DicomFile dicom(buffer.get(), size);
        ASSERT_TRUE(dicom.IsValid());
        std::cout << "DicomFile loaded." << std::endl;
        auto filtered = filter.ApplyFilter(dicom);
        ASSERT_TRUE(fs::exists(path.parent_path().string()));
        fs::path output_path(path.parent_path().string() + "/filtered.dcm");
        std::ofstream output(output_path);
        output.write(std::get<0>(filtered).get(),std::get<1>(filtered));
        ASSERT_TRUE(output.good());
        output.close();
        ASSERT_TRUE(fs::file_size(output_path) != 0);
        ASSERT_TRUE(fs::file_size(output_path) <= size);
        size = fs::file_size(output_path);
        buffer = std::unique_ptr<char[]>(new char[size]);
        file.open(output_path);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        dicom = DicomFile(buffer.get(), size);
        ASSERT_TRUE(dicom.IsValid());
        fs::remove(output_path);
    };
    test(GetProjRoot().string() + "/samples/0002.DCM");
    //TestWithDicomFiles(test);
    std::cout << "Test Complete." << std::endl;
}