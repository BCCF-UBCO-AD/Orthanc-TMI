#include <orthanc/OrthancCPlugin.h>

static OrthancPluginContext* context_ = nullptr;

extern "C" {
    int32_t OrthancPluginInitialize(OrthancPluginContext* context){

    }
    
    void OrthancPluginFinalize(){

    }

    const char* OrthancPluginGetName(){

    }
    
    const char* OrthancPluginGetVersion(){

    }
}