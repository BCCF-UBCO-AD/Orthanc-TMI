#pragma once
#include <core.h>
#include <dicom-anonymizer.h>

class PluginConfigurer {
private:
    static nlm::json config;
    static std::unordered_map<tag_uint64_t,std::string> date_formats;
    static nlm::json hardlinks;
protected:
public:
    static int Initialize();
    static void UnitTestInitialize(nlm::json &cfg);
    static std::string GetDateFormat(tag_uint64_t tag = 0);
    static const nlm::json& GetHardlinksJson();
};
