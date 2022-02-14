#include <dicom-file.h>
#include <dicom-element-view.h>
#include <vector>
#include <fstream>
#include <iostream>

DicomFile::DicomFile(const void* data, size_t size) {
    this->data = data;
    this->size = size;
    Parse();
}

DicomFile::DicomFile(size_t size) {
    buffer = std::unique_ptr<char[]>(new char[size]);
    is_const = false;
}

DicomFile& DicomFile::operator=(DicomFile &&other) noexcept {
    instance = other.instance;
    data = other.data;
    buffer = std::move(other.buffer);
    size = other.size;
    is_valid = other.is_valid;
    elements.clear();
    elements.swap(other.elements);
    other.data = nullptr;
    other.buffer.reset();
    return *this;
}

bool DicomFile::Parse(void* data, size_t size) {
    return DicomFile(data,size).IsValid();
}

bool DicomFile::Parse() {
    char msg_buffer[256] = {0};
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    DEBUG_LOG(1,"DicomFile: parsing dicom data");
    // there is no valid header if the file size is smaller than 132 bytes
    if(size <= preamble + prefix){
        DEBUG_LOG(PLUGIN_ERRORS,"DicomFile does not have a valid size, cannot continue parsing");
        is_valid = false;
        return false;
    }
    // the DICOM file header must end with DICM
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        DEBUG_LOG(PLUGIN_ERRORS,"DicomFile does not match a valid DICOM format, cannot continue parsing");
        is_valid = false;
        return false;
    }
    // parse dicom data elements
    size_t i;
    for(i = preamble+prefix; i < size;){
        // parse next element
        DicomElementView element(readable_buffer, i);
        // print element info
        sprintf(msg_buffer,"[%s] (%s,%s)->(%s)\n idx: %zu, next: %zu, size: %zu, value offset: %zu, length: %d",
                element.VR.c_str(),
                element.HexGroup().c_str(),
                element.HexElement().c_str(),
                element.HexTag().c_str(),
                element.idx,
                element.GetNextIndex(),
                element.size,
                element.value_offset,
                (int)element.value_length);
        DEBUG_LOG(2,msg_buffer);
        // save element range
        size_t j = element.GetNextIndex();
        // todo: should it be j-1?
        elements.emplace_back(element.tag, std::make_pair(i,j));
        i = j;
    }
    sprintf(msg_buffer,"buffer size = %ld, read head idx = %ld", size, i);
    DEBUG_LOG(1,msg_buffer);
    is_valid = i == size;
    return is_valid;
}

OrthancPluginErrorCode DicomFile::Write(const fs::path &master_path) {
    if(is_valid) {
        char msg[256] = {0};
        sprintf(msg, "DicomFile: writing to %s", master_path.c_str());
        DEBUG_LOG(0, msg);
        std::fstream file(master_path, std::ios::binary | std::ios::out);
        file.write((const char*) data, size);
        file.close();
        DEBUG_LOG(1, "DicomFile: write complete");
        if (!file.good()) {
            return OrthancPluginErrorCode_FileStorageCannotWrite;
        }
        // Set the permissions on the file we saved
        fs::file_status master_status = fs::status(master_path);
        master_status.permissions(globals::file_permissions);
        MakeHardlinks(master_path);
        return OrthancPluginErrorCode_Success;
    }
    DEBUG_LOG(PLUGIN_ERRORS, "DicomFile: invalid file, cannot write");
    return OrthancPluginErrorCode_CorruptedFile;
}

void DicomFile::MakeHardlinks(const fs::path &master_path){
    const fs::path storage_root(globals::storage_location);
    std::string uuid = master_path.filename();
    // create hard links
    auto hardlink_to = [&](std::string groupby, std::string group) {
        fs::path link = fs::path(storage_root)
                .append(groupby)
                .append(group)
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
    try {
        hardlink_to("/by-dob/", DOB_placeholder);
        hardlink_to("/by-patient-id/", PID_placeholder);
        //todo: also sort into actual/individual studies
        hardlink_to("/by-study-date/", SD_placeholder);
    } catch (const std::exception &e) {
        DEBUG_LOG(PLUGIN_ERRORS,"We failed to create hard links. They may already exist. OR the placeholders still aren't replaced.")
        std::cerr << e.what() << std::endl;
    }
}