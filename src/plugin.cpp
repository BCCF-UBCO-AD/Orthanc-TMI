#define IMPLEMENTS_GLOBALS
#include <core.h>
#include <configuration.h>
#include <storage-area.h>
#include <plugin-configure.h>

namespace fs = std::filesystem;
namespace globals {
    OrthancPluginContext *context = nullptr;
    std::string storage_location;
    fs::perms dir_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read | fs::perms::sticky_bit;
    fs::perms file_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read;
}

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
        if(PluginConfigurer::Initialize() != 0){
            return -1;
        }
        OrthancPluginRegisterStorageArea2(context, StorageCreateCallback, StorageReadWholeCallback,
                                          StorageReadRangeCallback, StorageRemoveCallback);
        //OrthancPluginRegisterIncomingDicomInstanceFilter(context, FilterCallback);
        return 0;
    }
}
