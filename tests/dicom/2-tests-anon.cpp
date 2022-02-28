#include "../common.h"
#include <gtest/gtest.h>
#include <plugin-configure.h>
#include <dicom-anonymizer.h>

#include <iostream>
#include <fstream>
#include <cstdio>

std::string static_config = "{  \"DataAnon\": {\n"
                            "    \"Hardlinks\": {\n"
                            "      \"/by-study-date/\": \"0008,0020\",\n"
                            "      \"/by-pid/\": \"0010,0020\",\n"
                            "      \"/by-dob/\": \"0010,0030\"\n"
                            "    },\n"
                            "    \"DateTruncation\": {\n"
                            "      \"default\": \"YYYY0101\",\n"
                            "      \"0010,0030\": \"YYY00101\"\n"
                            "    },\n"
                            "    \"Filter\": {\n"
                            "      \"blacklist\": [\n"
                            "        \"0010\"\n"
                            "      ],\n"
                            "      \"whitelist\": [\n"
                            "        \"0010,0030\"\n"
                            "      ]\n"
                            "    }\n"
                            "  }}";

class TestAnonymizer{
    static bool CheckDate(DicomElementView &view){
        std::string date = std::string(std::string_view(view.GetValueHead(),view.value_length));
        return date.find("0101") != std::string::npos;
    }
public:
    static bool CheckOutput(const DicomFile &dicom){
        auto &blacklist = DicomAnonymizer::blacklist;
        auto &whitelist = DicomAnonymizer::whitelist;
        for (const auto &[tag, range]: dicom.elements) {
            if (!whitelist.contains(tag)) {
                if(blacklist.contains(tag) || blacklist.contains(tag&GROUP_MASK)){
                    return false;
                }
            } else {
                auto &[start,end] = range;
                DicomElementView view(dicom.data, start);
                if (view.VR == "DA" && view.value_length != 0 && !CheckDate(view)){
                    return false;
                }
            }
        }
        return true;
    }
};

TEST(dicom, anonymize) {
    std::cout << "Unit Test: dicom anonymize" << std::endl;
    //fs::path config_path(GetProjRoot().string() + "/docker/orthanc/orthanc.json");
    nlm::json config = nlm::json::parse(static_config.c_str());
    std::cout << "Configuring.." << std::endl;
    PluginConfigurer::UnitTestInitialize(config);
    DicomAnonymizer::debug();
    DicomAnonymizer anon;
    std::cout << " configured!" << std::endl;

    size_t files_passed = 0;
    size_t files_total = 0;
    auto test = [&](const fs::path &path){
        char msg[1024] = {0};
        files_total++;
        // Allocate buffer
        auto size = fs::file_size(path);
        std::shared_ptr<char[]> buffer(new char[size]);

        // Load file into buffer
        sprintf(msg,"Loading %s\n", path.c_str());
        DEBUG_LOG(DEBUG_1,msg);
        std::ifstream file(path, std::ios::binary | std::ios::in);
        ASSERT_TRUE(file.is_open());
        file.read(buffer.get(),size);
        file.close();
        ASSERT_TRUE(file.good());

        // Parse file
        DicomFile dicom(buffer, size);
        ASSERT_TRUE(dicom.IsValid());

        // Anonymize file
        ASSERT_TRUE(anon.Anonymize(dicom));

        // Verify result
        ASSERT_TRUE(TestAnonymizer::CheckOutput(dicom));

        files_passed++;
    };
    //test(GetProjRoot().string() + "/samples/0002.DCM");
    TestWithDicomFiles(test);
    std::cout << files_passed << "/" << files_total << " files passed" << std::endl;
    std::cout << "Test Complete.\n" << std::endl;
}