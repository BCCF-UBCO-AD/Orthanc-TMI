#include <orthanc/OrthancCPlugin.h>
#include "configuration.h"

static OrthancPluginContext* context_ = nullptr;

OrthancPluginErrorCode OnStoredCallback(OrthancPluginDicomInstance* instance,
                                        const char* instanceId) {
    char buffer[1024];
    sprintf(buffer, "Just received a DICOM instance of size %d and ID %s from origin %d (AET %s)",
            (int) OrthancPluginGetInstanceSize(context_, instance), instanceId,
            OrthancPluginGetInstanceOrigin(context_, instance),
            OrthancPluginGetInstanceRemoteAet(context_, instance));
    OrthancPluginLogWarning(context_, buffer);

    char* json = OrthancPluginGetInstanceJson(context_, instance);
    OrthancPluginLogWarning(context_, json);
    OrthancPluginFreeString(context_, json);
    
    return OrthancPluginErrorCode_Success;
}

extern "C" {
    int32_t OrthancPluginInitialize(OrthancPluginContext* context){
        context_ = context;

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

        OrthancPluginRegisterOnStoredInstanceCallback(context_,
                                                      reinterpret_cast<OrthancPluginOnStoredInstanceCallback>(OnStoredCallback));
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