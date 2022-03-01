#pragma once
#include <core.h>
#include <dicom-anonymizer.h>

class PluginConfigurer {
private:
    static nlm::json config;
    static nlm::json hardlinks;
    static bool hardlinks_use_bins;
protected:
public:
    static int Initialize();
    static int Initialize_impl(nlm::json &cfg);
    static std::string GetDateFormat(tag_uint64_t tag = 0);
    static const nlm::json& GetHardlinksJson();
    static bool UseHashBins() { return hardlinks_use_bins; }
};
