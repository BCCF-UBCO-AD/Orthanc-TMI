#include <plugin-configure.h>
#include <dicom-anonymizer.h>
#include <db-interface.h>
#include <job-queue.h>
#include <iostream>

nlm::json PluginConfigurer::config;
nlm::json PluginConfigurer::hardlinks;
bool PluginConfigurer::hardlinks_use_bins = false;
char PluginConfigurer::connection_string[1024];

//https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-CONNSTRING
//postgresql://[userspec@][hostspec][/dbname][?paramspec]
//    where userspec is:
//     user[:password]
//    and hostspec is:
//     [host][:port][,...]
//    and paramspec is:
//     name=value[&...]
// ex:
//  postgresql://user:secret@localhost
//  postgresql://other@localhost/otherdb?connect_timeout=10&application_name=myapp
//  postgresql://host1:123,host2:456/somedb?target_session_attrs=any&application_name=myapp

int PluginConfigurer::InitializePlugin() {
    try {
        nlm::json cfg = nlm::json::parse(OrthancPluginGetConfiguration(globals::context));
        int status = Initialize_impl(cfg);
        if(status != 0) {
            return status;
        }
        DEBUG_LOG(DEBUG_2, "Initialize_impl passed, now to retrieve the database connection information and connect.");
        //todo: document these as requirements
        //todo: add secret option? presumably the PostgreSQL plugin supports it.. we shouldn't break if the user uses it
        std::string database = cfg["PostgreSQL"]["Database"].get<std::string>();
        std::string host = cfg["PostgreSQL"]["Host"].get<std::string>();
        uint16_t port = cfg["PostgreSQL"]["Port"].get<uint16_t>();
        std::string username = cfg["PostgreSQL"]["Username"].get<std::string>();
        std::string password = cfg["PostgreSQL"]["Password"].get<std::string>();
        sprintf(connection_string, "postgresql://%s:%s@%s:%hu/%s", username.c_str(), password.c_str(), host.c_str(), port, database.c_str());
        JobQueue::GetInstance().AddJob([](){
            DBInterface::Initialize();
        });
        //DBInterface::GetInstance().CreateTables();
    } catch (const std::exception &e) {
        DEBUG_LOG(PLUGIN_ERRORS,"Failed inside PluginConfigurer::InitializePlugin");
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
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
        DEBUG_LOG(PLUGIN_ERRORS,"Failed inside PluginConfigurer::Initialize_impl");
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}

const std::string& PluginConfigurer::GetDBConnectionInfo() {
    static std::string str(connection_string);
    return str;
}

std::string PluginConfigurer::GetDateFormat(tag_uint64_t tag) {
    auto dadt = config.at("DataAnon").at("DateTruncation");
    auto tag_key = HexToKey(DecToHex(tag, 4));
    if (dadt.contains(tag_key)) {
        return dadt.at(tag_key).get<std::string>();
    }
    return dadt.at("default").get<std::string>();
}
