#include "callbacks.h"
#include "dicom/dicom-checksum.h"

OrthancPluginErrorCode OnStoredInstanceCallback(const OrthancPluginDicomInstance *instance, const char *instanceId) {
    char msg[1024] = {0};
    sprintf(msg, "OnStored Instance Id: %s", instanceId);
    DEBUG_LOG(1, msg);
    const void* instance_data = OrthancPluginGetInstanceData(globals::context, instance);
    if (DicomChecksum::UpdateDatabase(instance_data)) {
        return OrthancPluginErrorCode_Success;
    }
    return OrthancPluginErrorCode_UnknownResource;
}