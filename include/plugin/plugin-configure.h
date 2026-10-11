#pragma once
#include <core.h>
#include <json.h>
#include <dicom-tag.h>

class PluginConfigurer {
private:
    static nlm::json config;
    static nlm::json hardlinks;
    static bool hardlinks_use_bins;
    static char connection_string[1024];
protected:
public:
    static int InitializePlugin();
    static int Initialize_impl(nlm::json &cfg);
    static bool UseHashBins() { return hardlinks_use_bins; }
    static const nlm::json& GetHardlinksJson() { return hardlinks; }
    static const std::string& GetDBConnectionInfo();
    static std::string GetDateFormat(tag_uint64_t tag = 0);
};
