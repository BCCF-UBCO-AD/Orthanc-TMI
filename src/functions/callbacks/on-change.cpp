#include <callbacks.h>
#include <db-interface.h>

OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType, OrthancPluginResourceType resourceType, const char *resourceId) {
    if (changeType == OrthancPluginChangeType::OrthancPluginChangeType_NewPatient) {
        char msg[1024] = {0};
        sprintf(msg, "New patient added: %s", resourceId);
        DEBUG_LOG(1, msg);
        DBInterface::GetInstance().UpdateCrosswalk(resourceId);
    }
    return OrthancPluginErrorCode_Success;
}