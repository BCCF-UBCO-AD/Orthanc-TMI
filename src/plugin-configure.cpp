#include <plugin-configure.h>
#include <iostream>

nlm::json PluginConfigurer::config;
DicomFilter PluginConfigurer::filter = DicomFilter::ParseConfig("");

int PluginConfigurer::Initialize() {
    try {
        config = nlm::json::parse(OrthancPluginGetConfiguration(globals::context));
        if (config["StorageDirectory"].is_string()) {
            globals::storage_location = config["StorageDirectory"].get<std::string>();
            fs::create_directories(globals::storage_location);
            fs::permissions(globals::storage_location, globals::dir_permissions);
        } else {
            DEBUG_LOG(PLUGIN_ERRORS,"Configuration json does not contain a StorageDirectory field.");
            return -1;
        }
        filter = DicomFilter::ParseConfig(config);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}