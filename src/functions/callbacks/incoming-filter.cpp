#include <core.h>
#include <dicom-file.h>
#include <dicom-anonymizer.h>
#include <data-transport.h>

int32_t FilterCallback(const OrthancPluginDicomInstance *instance) {
    DEBUG_LOG(DEBUG_1, "Filter: receiving dicom");
    const void* instance_data = OrthancPluginGetInstanceData(globals::context, instance);
    int64_t instance_size = OrthancPluginGetInstanceSize(globals::context, instance);
    DicomFile file(instance_data, instance_size);
    DicomAnonymizer anon;
    if(anon.Anonymize(file)) {
        if(!DataTransport::Emplace(file.CalculateMd5())){
            // this is a duplicate DICOM instance
            return 0;
        }
        DataTransport::Emplace(instance_data, file.CalculateMd5(), file.GetSize());
        return 1;
    }
    return -1; /*{0: discard, 1: store, -1: error}*/
}