#include <orthanc/OrthancCPlugin.h>
#include "configuration.h"
#include <pqxx/pqxx>

static OrthancPluginContext* context_ = nullptr;

extern "C" {
    int32_t OrthancPluginInitialize(OrthancPluginContext* context){
        context_ = context;
        pqxx::connection c("postgresql://postgres:example@localhost:5432");
        pqxx::work w(c);
        assert(c.is_open());
        if(!c.is_open()) return -1;

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