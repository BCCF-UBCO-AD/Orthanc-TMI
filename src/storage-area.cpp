#include <core.h>
#include <dicom-file.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

OrthancPluginErrorCode WriteDicomFile(DicomFile dicom, const char *uuid){
    static fs::path storage_root(globals::storage_location);
    char* content = nullptr;
    size_t size = 0;
    bool cleanup = false;
    if(dicom.IsValid()) {
        auto filtered = dicom.ApplyFilter(globals::filter_list);
        if (std::get<0>(filtered)) {
            content = std::get<0>(filtered);
            size = std::get<1>(filtered);
            if(size == 0){
                // todo: probably a better error code
                return OrthancPluginErrorCode_EmptyRequest;
            }
            cleanup = true;
        }
        // write to disk
        fs::path origin(storage_root.string() + "/by-uuid/");
        fs::create_directories(origin);
        // todo: add uuid directory portion
        std::fstream file(origin.append(uuid));

        // create hard links
//        fs::path dob(storage_root.string() + "/by-dob/");
//        fs::create_directories(dob);
//        fs::path dob_link(dob.append(uuid));
//
//        fs::path pid(storage_root.string() + "/by-patient-id/");
//        fs::create_directories(pid);
//        fs::path pid_link(pid.append(uuid));
        if(cleanup){
            delete[] content;
        }
    } else {
        return OrthancPluginErrorCode_BadFileFormat;
    }
}

OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                             const void *content,
                                             int64_t size,
                                             OrthancPluginContentType type) {
    DicomFile dicom;
    bool cleanup = false;
    switch(type){
        case OrthancPluginContentType_Dicom:
        case OrthancPluginContentType_Unknown:
            dicom = DicomFile(content,size);
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
    WriteDicomFile(dicom, uuid);
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
