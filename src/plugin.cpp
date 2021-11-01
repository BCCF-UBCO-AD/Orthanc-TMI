#define IMPLEMENTS_GLOBALS
#include "configuration.h"
#include "core.h"
#include "dicom-filter.h"
#include "db-interface.h"

#include <nlohmann/json.hpp>

namespace nlm = nlohmann;
namespace globals {
    OrthancPluginContext *context = nullptr;
    std::unordered_set<uint32_t> filter_list;
}

// prototypes
int32_t FilterCallback(const OrthancPluginDicomInstance* instance);
void PopulateFilterList(nlm::json config);
const char* ParseTag(const char* buffer, nlm::json config);

// plugin foundation
extern "C" {
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
        nlm::json config(OrthancPluginGetConfiguration(context));
        // todo 1: test that this password getting line works
        // todo 2: test what happens when the json doesn't have the necessary fields
        auto password = config["PostgreSQL"]["Password"].get<std::string>();
        DBInterface::connect(password.c_str());
        PopulateFilterList(config);
        OrthancPluginRegisterIncomingDicomInstanceFilter(context, FilterCallback);

        return 0;
    }
    
    void OrthancPluginFinalize(){

    }

    const char* OrthancPluginGetName(){
        return ORTHANC_PLUGIN_NAME;
    }
    
    const char* OrthancPluginGetVersion(){
        return ORTHANC_PLUGIN_VERSION;
    }
}

using namespace globals;

void PopulateFilterList(nlm::json config) {
    // todo: write a type safe loop, this thing will probably crash at runtime, since the tags won't be stored as integers
    for(uint32_t tag : config["filtered-tags"]){
        // todo: implement me, tags will be stored as strings they need to be converted (probably)
        filter_list.emplace(tag);
    }
}

int32_t FilterCallback(const OrthancPluginDicomInstance* instance){
    // todo: possibly copy instance data to new buffer to control life span, then anonymize as a job instead of in this callstack
    DicomFilter parser(instance);
    auto new_instance = parser.GetFilteredInstance();
    if (!new_instance) {
        return -1;
    }
    if (new_instance == instance) {
        return 1;
    }
    return 0; /*{0: discard, 1: store, -1: error}*/
}
