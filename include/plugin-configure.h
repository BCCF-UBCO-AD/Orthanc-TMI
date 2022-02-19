#pragma once
#include <core.h>
#include <dicom-anonymizer.h>

class PluginConfigurer {
private:
    static nlm::json config;
    static DicomAnonymizer filter;
    static std::unordered_map<tag_code,std::string> date_formats;
protected:
public:
    static int Initialize();
    static void UnitTestInitialize(nlm::json &cfg);
    static DicomAnonymizer& GetDicomFilter() { return filter; }
    static std::string GetDateFormat(uint64_t tag_code = 0);
};