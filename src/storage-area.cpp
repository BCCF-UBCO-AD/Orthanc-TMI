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
        fs::path master_path = fs::path(storage_root).append("/by-uuid/").append(uuid);
        fs::create_directories(master_path);
        // todo: add uuid directory portion
        std::fstream file(master_path, std::ios::binary | std::ios::out);
        file.write(content,size);
        file.close();
        // set file permissions
        // todo: permission debug info?
        fs::file_status master_status = fs::status(master_path);
        master_status.permissions(globals::file_permissions);

        // create hard links
        auto hardlink_to = [&](std::string group, std::string folder) {
            fs::path link = fs::path(storage_root)
                    .append(group)
                    .append(folder)
                    .append(uuid);
            fs::create_directories(link);
            fs::create_hard_link(master_path, link);
            fs::permissions(link, globals::file_permissions);
        };
        // todo: integrate json settings to enable/disable individual hard links
        // todo: replace placeholders
        std::string DOB_placeholder;
        std::string PID_placeholder;
        std::string SD_placeholder;
        hardlink_to("/by-dob/", DOB_placeholder);
        hardlink_to("/by-patient-id/", PID_placeholder);
        hardlink_to("/by-study-date/", SD_placeholder);
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
