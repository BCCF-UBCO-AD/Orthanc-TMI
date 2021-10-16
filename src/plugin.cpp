#include <orthanc/OrthancCPlugin.h>
#include <string>
#include "configuration.h"

static OrthancPluginContext *context_ = nullptr;

OrthancPluginErrorCode OnStoredCallback(OrthancPluginDicomInstance *instance, const char *instanceId) {
    char buffer[1024];
    sprintf(buffer, "Just received a DICOM instance of size %d and ID %s from origin %d (AET %s)",
            (int) OrthancPluginGetInstanceSize(context_, instance), instanceId,
            OrthancPluginGetInstanceOrigin(context_, instance),
            OrthancPluginGetInstanceRemoteAet(context_, instance));
    OrthancPluginLogWarning(context_, buffer);

    std::string instanceId_str = std::string(instanceId);

    OrthancPluginErrorCode error;

    OrthancPluginMemoryBuffer anonymized_buffer;
    std::string anonymize_uri = "/instances/" + instanceId_str + "/anonymize";
    std::string anonymize_body = "{}";
    error = OrthancPluginRestApiPost(context_, &anonymized_buffer, anonymize_uri.c_str(), anonymize_body.c_str(), anonymize_body.length());
    if (error) return error;

    OrthancPluginMemoryBuffer instance_buffer;
    std::string instance_uri = "/instances/" + instanceId_str;
    error = OrthancPluginRestApiGet(context_, &instance_buffer, instance_uri.c_str());
    if (error) return error;

    char anonymized_filename[1024];
    sprintf(anonymized_filename, "%s" ,(char *)instance_buffer.data);
    OrthancPluginLogWarning(context_, anonymized_filename);
    //error = OrthancPluginWriteFile(context_, anonymized_filename, anonymized_buffer.data, anonymized_buffer.size);
    if (error) return error;

    OrthancPluginFreeMemoryBuffer(context_, &instance_buffer);
    OrthancPluginFreeMemoryBuffer(context_, &anonymized_buffer);

    return OrthancPluginErrorCode_Success;
}

extern "C" {
int32_t OrthancPluginInitialize(OrthancPluginContext *context) {
    context_ = context;

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(context) == 0) {
        char info[256];
        sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
                context->orthancVersion,
                ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
                ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
                ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);

        OrthancPluginLogError(context, info);
        return -1;
    }
    auto isntance_id = reinterpret_cast<OrthancPluginOnStoredInstanceCallback>(OnStoredCallback);
    OrthancPluginRegisterOnStoredInstanceCallback(context, isntance_id);
    return 0;
}

void OrthancPluginFinalize() {

}

const char *OrthancPluginGetName() {
    return ORTHANC_PLUGIN_NAME;
}

const char *OrthancPluginGetVersion() {
    return ORTHANC_PLUGIN_VERSION;
}
}