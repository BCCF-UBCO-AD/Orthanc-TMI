#define IMPLEMENTS_GLOBALS
#include "configuration.h"
#include "core.h"
#include "dicom-filter.h"

#include <nlohmann/json.hpp>

namespace nlm = nlohmann;
namespace globals {
    OrthancPluginContext *context = nullptr;
    std::unordered_set<uint32_t> filter_list;
}

// prototypes
int32_t FilterCallback(const OrthancPluginDicomInstance* instance);
void PopulateFilterList();

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
        PopulateFilterList();
        return OrthancPluginRegisterIncomingDicomInstanceFilter(context, FilterCallback);
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

void PopulateFilterList(){
    nlm::json config(OrthancPluginGetConfiguration(context));
    // todo: write a type safe loop, this thing will probably crash at runtime, since the tags won't be stored as integers
    for(auto iter : config["Dicom-Filter"]["tags"]){
        auto tag = iter.get<std::string>();
        // todo: implement me, tags will be stored as strings they need to be converted (probably)
        uint32_t tag_code = *(uint32_t*)(tag.c_str());
        char msg_buffer[256] = {0};
        sprintf(msg_buffer, "%d\n", tag_code);
        OrthancPluginLogInfo(context, msg_buffer);
        filter_list.emplace(tag_code);
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
