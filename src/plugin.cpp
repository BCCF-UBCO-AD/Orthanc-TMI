#define IMPLEMENTS_GLOBALS
#include <core.h>
#include <dicom-file.h>
#include <db-interface.h>
#include <nlohmann/json.hpp>
#include <configuration.h>
#include <storage-area.h>
#include <on-stored.h>
#include <plugin-configure.h>
#include <job-queue.h>
#include <thread>

namespace fs = std::filesystem;
namespace globals {
    OrthancPluginContext *context = nullptr;
    std::string storage_location;
    fs::perms dir_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read | fs::perms::sticky_bit;
    fs::perms file_permissions = fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read;
}

static std::thread job_thread;

// plugin foundation
extern "C" {
    void OrthancPluginFinalize(){
        JobQueue::GetInstance().Stop();
        job_thread.join();
    }
    const char* OrthancPluginGetName(){ return ORTHANC_PLUGIN_NAME; }
    const char* OrthancPluginGetVersion(){ return ORTHANC_PLUGIN_VERSION; }

    int32_t OrthancPluginInitialize(OrthancPluginContext* context){
        globals::context = context;
        /* Connect with database interface. */
        DBInterface::connect("postgres", "example");
        if(!DBInterface::is_open()){
            OrthancPluginLogError(context, "DBInterface failed to connect to DB.");
            return -1;
        }else{
            OrthancPluginLogError(context, "DBInterface connect success.");
            DBInterface::create_tables();
        }

        /* Check the version of the Orthanc core */
        if (OrthancPluginCheckVersion(context) == 0){
            char info[256];
            sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
                    context->orthancVersion,
                    ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
                    ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
                    ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
            DEBUG_LOG(PLUGIN_ERRORS,info);
            return -1;
        }
        if(PluginConfigurer::Initialize() != 0){
            return -1;
        }
        OrthancPluginRegisterStorageArea2(context, StorageCreateCallback, StorageReadWholeCallback,
                                          StorageReadRangeCallback, StorageRemoveCallback);
        OrthancPluginRegisterOnStoredInstanceCallback(context, OnStoredInstanceCallback);
        //OrthancPluginRegisterIncomingDicomInstanceFilter(context, FilterCallback);
        job_thread = std::thread(&JobQueue::Process, &JobQueue::GetInstance());
        return 0;
    }
}
