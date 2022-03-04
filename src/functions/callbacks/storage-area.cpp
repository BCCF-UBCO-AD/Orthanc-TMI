#include <dicom-file.h>
#include <data-transport.h>
#include <dicom-anonymizer.h>
//#include <db-interface.h>
//#include <job-queue.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

const fs::path GetPath(OrthancPluginContentType type, const char* uuid){
    const static std::string storage_root(globals::storage_location);
    fs::path path;
    std::string b1 = std::string(std::string_view(uuid,3)) + "/";
    std::string b2 = std::string(std::string_view(uuid+3,2)) + "/";
    switch(type){
        case OrthancPluginContentType_Dicom:
            path = fs::path(storage_root).string()
                    .append("/by-uuid/")
                    .append(b1)
                    .append(b2)
                    .append(uuid)
                    .append(".DCM");
            break;
        case OrthancPluginContentType_DicomAsJson:
            path = fs::path(storage_root).string()
                    .append("/json/")
                    .append(b1)
                    .append(b2)
                    .append(uuid)
                    .append(".json");
            break;
        case OrthancPluginContentType_DicomUntilPixelData:
            path = fs::path(storage_root).string()
                    .append("/no-pixel/")
                    .append(b1)
                    .append(b2)
                    .append(uuid)
                    .append(".DCM");
            break;
        case _OrthancPluginContentType_INTERNAL:
            path = fs::path(storage_root).string()
                    .append("/internal/")
                    .append(b1)
                    .append(b2)
                    .append(uuid);
            break;
        case OrthancPluginContentType_Unknown:
            path = fs::path(storage_root).string()
                    .append("/unknown-files/")
                    .append(b1)
                    .append(b2)
                    .append(uuid);
            break;
    }
    return path;
}

OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                             const void *content,
                                             int64_t size,
                                             OrthancPluginContentType type) {
    char msg[1024] = {0};
    sprintf(msg,"StorageCreateCallback:\nuuid: %s\ncontent: %zu\n", uuid, (size_t)content);
    DEBUG_LOG(DEBUG_1,msg)
    fs::path path = GetPath(type, uuid);
    switch (type) {
        case OrthancPluginContentType_Dicom: {
            DicomFile file = DataTransport::PopFile(content);
            DataTransport::Emplace(content, uuid);
            fs::create_directories(path.parent_path());
            return file.Write(path);
        }
        // todo: do these require special processing? or is the content correct already?
        case OrthancPluginContentType_Unknown:
        case OrthancPluginContentType_DicomAsJson:
        case OrthancPluginContentType_DicomUntilPixelData:
        case _OrthancPluginContentType_INTERNAL:
            break;
    }
    std::fstream file(path, std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.write((const char *) content, size);
        if (file.good()) {
            file.close();
            return OrthancPluginErrorCode_Success;
        }
        DEBUG_LOG(PLUGIN_ERRORS,"StorageCreateCallback: but write out appears bad");
    }
    return OrthancPluginErrorCode_FileStorageCannotWrite;
}

OrthancPluginErrorCode StorageReadWholeCallback(OrthancPluginMemoryBuffer64 *target,
                                                const char *uuid,
                                                OrthancPluginContentType type) {
    auto path = GetPath(type,uuid);
    OrthancPluginCreateMemoryBuffer64(globals::context, target, fs::file_size(path));
    std::fstream file(path, std::ios::in | std::ios::binary);
    if(file.is_open()){
        file.read((char*)target->data, target->size);
        if(file.good()){
            file.close();
            return OrthancPluginErrorCode_Success;
        }
        DEBUG_LOG(PLUGIN_ERRORS,"StorageReadWholeCallback: opened file, but couldn't read it");
        return OrthancPluginErrorCode_StorageAreaPlugin;
    }
    return OrthancPluginErrorCode_InexistentFile;
}

OrthancPluginErrorCode StorageReadRangeCallback(OrthancPluginMemoryBuffer64 *target,
                                                const char *uuid,
                                                OrthancPluginContentType type,
                                                uint64_t rangeStart) {
    auto path = GetPath(type,uuid);
    OrthancPluginCreateMemoryBuffer64(globals::context, target, fs::file_size(path));
    std::fstream file(path, std::ios::in | std::ios::binary);
    if(file.is_open()){
        file.seekg(rangeStart);
        file.read((char*)target->data, target->size);
        return OrthancPluginErrorCode_Success;
    }
    return OrthancPluginErrorCode_InexistentFile;
}

OrthancPluginErrorCode StorageRemoveCallback(const char *uuid, OrthancPluginContentType type) {
    switch(type){
        case OrthancPluginContentType_Dicom:
            // todo: rewrite
            const fs::path storage_root(globals::storage_location);
            auto remove = [&](std::string group, std::string folder) {
                fs::path path = fs::path(storage_root)
                        .append(group)
                        .append(folder)
                        .append(uuid)
                        .append(".DCM");
                if(fs::exists(path)){
                    fs::remove(path);
                }
            };
            // todo: replace placeholders, this requires database integration
            // todo: follow lead of MakeHardlinks() for implementation of this, ie. rewrite the removal code below
            std::string DOB_placeholder;
            std::string PID_placeholder;
            std::string SD_placeholder;
            remove("/by-dob/", DOB_placeholder);
            remove("/by-patient-id/", PID_placeholder);
            remove("/by-study-date/", SD_placeholder);
            break;
    }
    auto path = GetPath(type,uuid);
    if(fs::exists(path)) {
        fs::remove(path);
    }

    return OrthancPluginErrorCode_Success;
}
