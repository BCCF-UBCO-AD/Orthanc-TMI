#include <orthanc/OrthancCPlugin.h>
#include <nlohmann/json.hpp>
#include "configuration.h"

#include <string>

namespace globals {
    static OrthancPluginContext *context = nullptr;
}

// prototypes
int32_t FilterCallback(const OrthancPluginDicomInstance* instance);
const OrthancPluginDicomInstance* Anonymize(const OrthancPluginDicomInstance* readonly_instance);

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

int32_t FilterCallback(const OrthancPluginDicomInstance* instance){
    // todo: possibly copy instance data to new buffer to control life span, then anonymize as a job instead of in this callstack
    auto new_instance = Anonymize(instance);
    if (!new_instance) {
        return -1;
    }
    if (new_instance == instance) {
        return 1;
    }
    return 0; /*{0: discard, 1: store, -1: error}*/
}
//#include <cstdlib>
const OrthancPluginDicomInstance* Anonymize(const OrthancPluginDicomInstance* readonly_instance){
    const void* data = OrthancPluginGetInstanceData(context, readonly_instance);
    int64_t size = OrthancPluginGetInstanceSize(context, readonly_instance);
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    if(size <= preamble + prefix){
        // throw error, or something
    }
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        // throw error, or something
    }
    // move the read head to where the tag data begins
    readable_buffer = readable_buffer+preamble+prefix;
    nlohmann::basic_json config(OrthancPluginGetConfiguration(context));
    // todo: define config format expectations
    // read config, store group+element strings: data-filter
    // parse data, check data-filter for each new tag

    // todo: read file data, tag by tag, printing each tag and its various fields on new lines
    //    for(int64_t i = 0; i < size; ++i){
    //        printf("%s \n", tag_line);
    //    }
    bool no_removable_data = false;

    // compile new dicom file {header, image}
    // perform PHI lookup
    // if not found, insert PHI
    // if potential match found link records
    if(no_removable_data){
        return readonly_instance;
    }

    return nullptr;
}
