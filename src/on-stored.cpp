#include <core.h>
#include <dicom-checksum.h>

OrthancPluginErrorCode OnStoredInstanceCallback(const OrthancPluginDicomInstance *instance, const char *instanceId) {
    char msg[1024] = {0};
    sprintf(msg, "OnStored Instance Id: %s", instanceId);
    //update_checksum(instanceId);
    DEBUG_LOG(0,msg);
    DicomChecksum::update_checksum(instanceId);


    /*
    const void* instance_data = OrthancPluginGetInstanceData(globals::context, instance);
    int size = OrthancPluginGetInstanceSize(globals::context, instance);
    // update checksum - We need both md5 and size;
    char* md5 = OrthancPluginComputeMd5(globals::context, instance_data, size);

    memset(msg, 0, sizeof msg);
    sprintf(msg, "Checksum: MD5 = %s, size = %d", md5, size);
    DEBUG_LOG(1,msg);
    */
    //DBInterface::UpdateChecksum(uuid, size, md5);
    //DEBUG_LOG(1,"Updated checksum");
    return OrthancPluginErrorCode_Success;
}