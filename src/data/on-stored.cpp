#include "core.h"
#include "dicom/dicom-checksum.h"

OrthancPluginErrorCode OnStoredInstanceCallback(const OrthancPluginDicomInstance *instance, const char *instanceId) {
    char msg[1024] = {0};
    sprintf(msg, "OnStored Instance Id: %s", instanceId);
    DEBUG_LOG(1,msg);

    const void* instance_data = OrthancPluginGetInstanceData(globals::context, instance);
    auto iter = DicomChecksum::checksum_map.find(instance_data);

    if (iter == DicomChecksum::checksum_map.end()) {
        // Checksum not found in checksum_map
        sprintf(msg, "OnStored Instance Id: %s", instanceId);
        DEBUG_LOG(1,"Checksum not found!");
        return OrthancPluginErrorCode_UnknownResource;
    }

    auto checksum = iter->second;
    sprintf(msg, "UUID: %s, MD5: %s", std::get<0>(checksum), std::get<1>(checksum));
    DEBUG_LOG(1,msg);

    DicomChecksum::checksum_map.erase(iter);
    return OrthancPluginErrorCode_Success;
}