#include <core.h>
#include <dicom-file.h>
#include <filesystem>

namespace fs = std::filesystem;

OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                             const void *content,
                                             int64_t size,
                                             OrthancPluginContentType type) {
    DicomFile dicom;
    switch(type){
        case OrthancPluginContentType_Dicom:
        case OrthancPluginContentType_Unknown:
            dicom = DicomFile(content,size);
            if(dicom.IsValid()) {
                auto filtered = dicom.ApplyFilter(globals::filter_list);
                if (std::get<0>(filtered)) {
                    content = std::get<0>(filtered);
                    size = std::get<1>(filtered);
                }
            }
            break;
        case OrthancPluginContentType_DicomAsJson:
            //todo: implement DicomFile json constructor
            break;
        case OrthancPluginContentType_DicomUntilPixelData:
            // todo: will the plugin need to handle this?
            break;
        case _OrthancPluginContentType_INTERNAL:
            // todo: anything?
            break;
    }
    return OrthancPluginErrorCode_BadFileFormat;
}

OrthancPluginErrorCode StorageReadWholeCallback(OrthancPluginMemoryBuffer64 *target,
                                                const char *uuid,
                                                OrthancPluginContentType type) {
    return OrthancPluginErrorCode_BadFileFormat;
}

OrthancPluginErrorCode StorageReadRangeCallback(OrthancPluginMemoryBuffer64 *target,
                                                const char *uuid,
                                                OrthancPluginContentType type,
                                                uint64_t rangeStart) {
    return OrthancPluginErrorCode_BadFileFormat;
}

OrthancPluginErrorCode StorageRemoveCallback(const char *uuid, OrthancPluginContentType type) {
    return OrthancPluginErrorCode_InexistentFile;
}
