#define IMPLEMENTS_GLOBALS
#include <configuration.h>
#include <core.h>
#include <dicom-tag.h>

#include <nlohmann/json.hpp>

namespace nlm = nlohmann;
namespace fs = std::filesystem;
namespace globals {
    OrthancPluginContext *context = nullptr;
    TagFilter filter_list;
    std::string storage_location;
    fs::perms dir_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read | fs::perms::sticky_bit;
    fs::perms file_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read;
}

void PopulateFilterList(const nlm::json &config){
    // iterate the Dicom-Filter configuration tags array
    for (const auto &iter: config["Dicom-Filter"]["tags"]) {
        // todo: check that the string has 9 characters; true: do below, false: implement full group filters (eg. "0002,*", "0002")
        // get tag string, convert to decimal
        auto tag = iter.get<std::string>();
        tag.append(tag.substr(0, 4));
        tag.erase(0, 5);
        uint32_t tag_code = HexToDec(tag);
        // register tag
        globals::filter_list.emplace(tag_code);
        // log the tags registered
        char msg_buffer[256] = {0};
        sprintf(msg_buffer, "filter registered tag code: %d", tag_code);
        OrthancPluginLogWarning(globals::context, msg_buffer);
    }
}

// storage callback prototypes
extern OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                                    const void *content,
                                                    int64_t size,
                                                    OrthancPluginContentType type);
extern OrthancPluginErrorCode StorageReadWholeCallback(OrthancPluginMemoryBuffer64 *target,
                                                       const char *uuid,
                                                       OrthancPluginContentType type);
extern OrthancPluginErrorCode StorageReadRangeCallback(OrthancPluginMemoryBuffer64 *target,
                                                       const char *uuid,
                                                       OrthancPluginContentType type,
                                                       uint64_t rangeStart);
extern OrthancPluginErrorCode StorageRemoveCallback(const char *uuid, OrthancPluginContentType type);

// plugin foundation
extern "C" {
    void OrthancPluginFinalize(){}
    const char* OrthancPluginGetName(){ return ORTHANC_PLUGIN_NAME; }
    const char* OrthancPluginGetVersion(){ return ORTHANC_PLUGIN_VERSION; }

    int32_t OrthancPluginInitialize(OrthancPluginContext* context){
        globals::context = context;
        /* Check the version of the Orthanc core */
        if (OrthancPluginCheckVersion(context) == 0){
            char info[256];
            sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
                    context->orthancVersion,
                    ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
                    ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
                    ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);

            OrthancPluginLogError(context, info);
            return -1;
        }
        const nlm::json config = nlm::json::parse(OrthancPluginGetConfiguration(context));
        if(config["StorageDirectory"].is_string()) {
            globals::storage_location = config["StorageDirectory"].get<std::string>();
            fs::create_directories(globals::storage_location);
            fs::permissions(globals::storage_location, globals::dir_permissions);
        } else {
            OrthancPluginLogError(context, "Configuration json does not contain a StorageDirectory field.");
            return -1;
        }
        PopulateFilterList(config);
        OrthancPluginRegisterStorageArea2(context, StorageCreateCallback, StorageReadWholeCallback,
                                          StorageReadRangeCallback, StorageRemoveCallback);
        //OrthancPluginRegisterIncomingDicomInstanceFilter(context, FilterCallback);
        return 0;
    }
}
