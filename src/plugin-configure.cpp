#include <plugin-configure.h>
#include <dicom-tag.h>
#include <iostream>

nlm::json PluginConfigurer::config;
DicomAnonymizer PluginConfigurer::filter;

int PluginConfigurer::Initialize() {
    try {
        config = nlm::json::parse(OrthancPluginGetConfiguration(globals::context));
        if (config["StorageDirectory"].is_string()) {
            globals::storage_location = config["StorageDirectory"].get<std::string>();
            fs::create_directories(globals::storage_location);
            fs::permissions(globals::storage_location, globals::dir_permissions);
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "Configuration json does not contain a StorageDirectory field.");
            return -1;
        }
        DicomAnonymizer::Configure(config);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}

void PluginConfigurer::UnitTestInitialize(nlm::json &cfg) {
    try {
        config = cfg;
        if (cfg["StorageDirectory"].is_string()) {
            globals::storage_location = cfg["StorageDirectory"].get<std::string>();
            fs::create_directories(globals::storage_location);
            fs::permissions(globals::storage_location, globals::dir_permissions);
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "Configuration json does not contain a StorageDirectory field.");
            return;
        }
        DicomAnonymizer::Configure(cfg);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

std::string PluginConfigurer::GetDateFormat(uint64_t tag_code) {
    //std::cout << config.dump(4) << std::endl;
    auto dt = config.at("Dicom-DateTruncation");
    auto tag_key = HexToKey(DecToHex(tag_code, 4));
    if (dt.contains(tag_key)) {
        return dt.at(tag_key).get<std::string>();
    }
    return dt.at("default").get<std::string>();
    // todo: change config format to have: default truncation format, and tag_code keyed formats so that this can be tested
}
