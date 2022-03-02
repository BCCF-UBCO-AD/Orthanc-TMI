#include <plugin-configure.h>
#include <dicom-tag.h>
#include <iostream>

nlm::json PluginConfigurer::config;
nlm::json PluginConfigurer::hardlinks;
bool PluginConfigurer::hardlinks_use_bins = false;

int PluginConfigurer::Initialize() {
    try {
        nlm::json cfg = nlm::json::parse(OrthancPluginGetConfiguration(globals::context));
        return Initialize_impl(cfg);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

int PluginConfigurer::Initialize_impl(nlm::json &cfg) {
    config = cfg;
    try {
        std::stringstream ss;
        ss << config.dump(2);
        DEBUG_LOG(DEBUG_2,ss.str().c_str());
        auto status = DicomAnonymizer::Configure(config);
        if (status != 0) return status;
        auto da_cfg = config.at("DataAnon");
        hardlinks = da_cfg.at("Hardlinks");
        if (config["StorageDirectory"].is_string()) {
            globals::storage_location = config["StorageDirectory"].get<std::string>();
            fs::create_directories(globals::storage_location);
            fs::permissions(globals::storage_location, globals::dir_permissions);
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "Configuration json does not contain a valid StorageDirectory field.");
            return -1;
        }
        if (da_cfg["HardlinksUseHashBins"].is_boolean()){
            hardlinks_use_bins = da_cfg["HardlinksUseHashBins"].get<bool>();
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "Configuration json does not contain a valid HardlinksUseHashBins field.");
            return -1;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}

std::string PluginConfigurer::GetDateFormat(tag_uint64_t tag) {
    auto dadt = config.at("DataAnon").at("DateTruncation");
    auto tag_key = HexToKey(DecToHex(tag, 4));
    if (dadt.contains(tag_key)) {
        return dadt.at(tag_key).get<std::string>();
    }
    return dadt.at("default").get<std::string>();
}
const nlm::json& PluginConfigurer::GetHardlinksJson() {
    return hardlinks;
}
