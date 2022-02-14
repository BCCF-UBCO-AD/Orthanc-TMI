#include "common.h"
#include <gtest/gtest.h>
#include <plugin-configure.h>
#include <dicom-anonymizer.h>
#include <iostream>
#include <fstream>

#include <cstdio>

std::string static_config = "{\n"
                            "  \"Dicom-DateTruncation\": {\n"
                            "    \"default\": [\"YYYY\",\"01\",\"01\"]\n"
                            "  },\n"
                            "  \"Dicom-Filter\": {\n"
                            "    \"blacklist\": [\n"
                            "      \"0010\"\n"
                            "    ],\n"
                            "    \"whitelist\": [\n"
                            "      \"0010,0030\"\n"
                            "    ]\n"
                            "  }\n"
                            "}";

class TestAnonymizer{
    static bool CheckDate(DicomElementView &view){
        std::string date = std::string(std::string_view(view.GetValueHead(),view.value_length));
        return date.find("0101") != std::string::npos;
    }
public:
    static bool CheckOutput(const DicomFile &dicom){
        auto &blacklist = DicomAnonymizer::blacklist;
        auto &whitelist = DicomAnonymizer::whitelist;
        for (const auto &[tag_code, range]: dicom.elements) {
            if (!whitelist.contains(tag_code)) {
                if(blacklist.contains(tag_code) || blacklist.contains(tag_code&GROUP_MASK)){
                    return false;
                }
            } else {
                auto &[start,end] = range;
                DicomElementView view(dicom.data, start);
                if (view.VR == "DA" && !CheckDate(view)){
                    return false;
                }
            }
        }
        return true;
    }
};

TEST(filtering, store_filtered) {
    //fs::path config_path(GetProjRoot().string() + "/docker/orthanc/orthanc.json");
    nlm::json config = nlm::json::parse(static_config.c_str());
    std::cout << "Configuring.." << std::endl;
    PluginConfigurer::UnitTestInitialize(config);
    DicomAnonymizer &anonymizer = PluginConfigurer::GetDicomFilter();
    std::cout << " configured!" << std::endl;

    size_t files_passed = 0;
    size_t files_total = 0;
    auto test = [&](const fs::path &path){
        char msg[1024] = {0};
        files_total++;
        auto size = fs::file_size(path);
        std::unique_ptr<char[]> buffer(new char[size]);

        sprintf(msg,"Loading %s\n", path.c_str());
        DEBUG_LOG(1,msg);
        std::ifstream file(path, std::ios::binary | std::ios::in);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());
        DicomFile dicom(buffer.get(), size); // valid
        ASSERT_TRUE(dicom.IsValid()); // pass
        sprintf(msg,"%s file\n",(dicom.IsValid() ? "valid" : "invalid"));
        DEBUG_LOG(1,msg);

        anonymizer.Anonymize(dicom); //memcpy ran til end of file(buffer)
        sprintf(msg,"Filtering: %s\n", (dicom.IsValid() ? "passed" : "failed"));
        DEBUG_LOG(1,msg);
        ASSERT_TRUE(dicom.IsValid()); //first sign the memcpy didn't work

        /*todo:
         * (1) check for blacklist && !whitelist tags
         * (2) check DOB truncation
         */
        files_passed++;
    };
    //test(GetProjRoot().string() + "/samples/0002.DCM");
    TestWithDicomFiles(test);
    std::cout << files_passed << "/" << files_total << " files passed" << std::endl;
    std::cout << "Test Complete." << std::endl;
}