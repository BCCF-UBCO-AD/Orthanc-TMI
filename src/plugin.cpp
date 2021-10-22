#include <orthanc/OrthancCPlugin.h>
#include "configuration.h"

namespace globals {
    static OrthancPluginContext *context = nullptr;
}

// prototypes
int32_t FilterCallback(const OrthancPluginDicomInstance* instance);
OrthancPluginDicomInstance* Anonymize(const OrthancPluginDicomInstance* instance);

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

int32_t FilterCallback(const OrthancPluginDicomInstance* instance){
    // todo: possibly copy instance data to new buffer to control life span, then anonymize as a job instead of in this callstack
    auto new_instance = Anonymize(instance);
    return 0; /*{0: discard, 1: store, -1: error}*/
}

//#include <cstdlib>
OrthancPluginDicomInstance* Anonymize(const OrthancPluginDicomInstance* instance){
    using namespace globals;
    const void* data = OrthancPluginGetInstanceData(context, instance);
    int64_t size = OrthancPluginGetInstanceSize(context, instance);
    const char* readable_buffer = (const char*)data;
    //    for(int64_t i = 0; i < size; ++i){
    //        printf("%c", readable_buffer[i]);
    //    }

    // extract header
    // extract PHI
    // extract images
    // compile new dicom file {header, image}
    // perform PHI lookup
    // if not found, insert PHI
    // if potential match found link records

    return nullptr;
}
