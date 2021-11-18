#include <core.h>
#include <dicom-file.h>
#include <db-interface.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

OrthancPluginErrorCode WriteDicomFile(DicomFile dicom, const char *uuid){
    static fs::path storage_root(globals::storage_location);
    char* content = nullptr;
    size_t size = 0;
    bool cleanup = false;
    if(dicom.IsValid()) {
        DBInterface::HandlePHI(dicom);
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
        fs::path master_path = fs::path(storage_root)
                .append("/by-uuid/")
                .append(uuid)
                .append(".DCM");
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
                    .append(uuid)
                    .append(".DCM");
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
    }
    return OrthancPluginErrorCode_BadFileFormat;
}

OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                             const void *content,
                                             int64_t size,
                                             OrthancPluginContentType type) {
    static fs::path storage_root(globals::storage_location);
    fs::path master_path;
    std::fstream file;
    switch(type) {
        case OrthancPluginContentType_Dicom:
            return WriteDicomFile(DicomFile(content, size), uuid);
        case OrthancPluginContentType_DicomAsJson:
            //todo: implement DicomFile json parser? or write a json file to disk?
            master_path = fs::path(storage_root)
                    .append("/json/")
                    .append(uuid)
                    .append(".json");
            file.open(master_path, std::ios::out | std::ios::binary);
            if (file.is_open()) {
                file.write((const char *) content, size);
                file.close();
            }
            return file.good() ?
                   OrthancPluginErrorCode_Success :
                   OrthancPluginErrorCode_CannotWriteFile;
        case OrthancPluginContentType_DicomUntilPixelData:
            // todo: will the plugin need to handle this? what does it look like and its use case?
            master_path = fs::path(storage_root)
                    .append("/no-pixel/")
                    .append(uuid)
                    .append(".DCM");
            file.open(master_path, std::ios::out | std::ios::binary);
            if (file.is_open()) {
                file.write((const char *) content, size);
                file.close();
            }
            return file.good() ?
                   OrthancPluginErrorCode_Success :
                   OrthancPluginErrorCode_CannotWriteFile;
        case _OrthancPluginContentType_INTERNAL:
            // todo: anything? what even is this?
            master_path = fs::path(storage_root)
                    .append("/internal/")
                    .append(uuid);
            file.open(master_path, std::ios::out | std::ios::binary);
            if (file.is_open()) {
                file.write((const char *) content, size);
                file.close();
            }
            return file.good() ?
                   OrthancPluginErrorCode_Success :
                   OrthancPluginErrorCode_CannotWriteFile;
        case OrthancPluginContentType_Unknown:
            //todo: write plain jane fstream write?
            master_path = fs::path(storage_root)
                    .append("/unknown-files/")
                    .append(uuid);
            file.open(master_path, std::ios::out | std::ios::binary);
            if (file.is_open()) {
                file.write((const char *) content, size);
                file.close();
            }
            return file.good() ?
                   OrthancPluginErrorCode_Success :
                   OrthancPluginErrorCode_CannotWriteFile;
    }
    return OrthancPluginErrorCode_EmptyRequest;
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
