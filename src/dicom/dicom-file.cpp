#include <dicom-file.h>
#include <dicom-element-view.h>
#include <plugin-configure.h>
#include <fstream>
#include <iostream>

DicomFile::DicomFile(std::shared_ptr<char[]> &buffer, size_t size) {
    this->size = size;
    this->buffer = buffer;
    data = this->buffer.get();
    elements.clear();
    Parse();
}

DicomFile::DicomFile(const void* data, size_t size) {
    this->data = data;
    this->size = size;
    elements.clear();
    buffer.reset();
    Parse();
}

bool DicomFile::Parse(void* data, size_t size) {
    return DicomFile(data,size).IsValid();
}

bool DicomFile::Parse() {
    char msg_buffer[256] = {0};
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    DEBUG_LOG(DEBUG_1,"DicomFile: parsing dicom data");
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
        std::string tag_key = HexToKey(DecToHex(element.tag));
        const char* kc = tag_key.c_str();
        sprintf(msg_buffer,"[%s] (%s)->(%d)\n idx: %zu, next: %zu, size: %zu, value offset: %zu, length: %d",
                element.VR.c_str(),
                kc,
                element.tag,
                element.idx,
                element.GetNextIndex(),
                element.size,
                element.value_offset,
                (int)element.value_length);
        DEBUG_LOG(DEBUG_2,msg_buffer);
        // save element range
        size_t j = element.GetNextIndex();
        elements.emplace_back(element.tag, std::make_pair(i,j));
        i = j;
    }
    sprintf(msg_buffer,"buffer size = %ld, read head idx = %ld", size, i);
    DEBUG_LOG(DEBUG_1,msg_buffer);
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
        DEBUG_LOG(DEBUG_1, "DicomFile: write complete");
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

void DicomFile::MakeHardlinks(const fs::path &master_path) {
    const static std::string storage_root(globals::storage_location);
    std::string uuid = master_path.filename();
    // create hard links
    auto hardlink_to = [&](std::string groupby, std::string group) {
        fs::path link;
        if (PluginConfigurer::UseHashBins()) {
            std::string b1 = std::string(std::string_view(uuid.c_str(), 2)) + "/";
            link = fs::path(storage_root).string()
                    .append(groupby)
                    .append(group)
                    .append(b1)
                    .append(uuid);
        } else {
            link = fs::path(storage_root).string()
                    .append(groupby)
                    .append(group)
                    .append(uuid);
        }
        char msg[1024];
        sprintf(msg, "storage root: %s", storage_root.c_str());
        DEBUG_LOG(DEBUG_1,msg);
        sprintf(msg,"Attempting to link %s -> %s", link.c_str(), master_path.c_str());
        DEBUG_LOG(DEBUG_1,msg);
        fs::create_directories(link.parent_path());
        fs::create_hard_link(master_path, link);
        fs::permissions(link, globals::file_permissions);
    };
    for (auto &[group_by, tag_key]: PluginConfigurer::GetHardlinksJson().items()) {
        auto tag = HexToDec(KeyToHex(tag_key));
        try {
            std::string group = GetElementValue(tag) + "/";
            // We're not going to make a link if the data is blank
            if (group.find_first_not_of(' ') != std::string::npos) {
                hardlink_to(group_by, group);
            } else {
                char msg[1024];
                sprintf(msg, "Cannot group a hardlink of this DicomFile by (%s) because that element is empty or nonexistent.", tag_key.get<std::string>().c_str());
                DEBUG_LOG(PLUGIN_ERRORS, msg);
            }
        } catch (const std::exception &e) {
            DEBUG_LOG(PLUGIN_ERRORS, "We failed to create a hard link. It may already exist.");
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string DicomFile::CalculateMd5() {
    if (calculated){
        return md5;
    }
    md5 = OrthancPluginComputeMd5(globals::context, data, size);
    calculated = true;
    return md5;
}

std::string DicomFile::GetElementValue(tag_uint64_t tag) {
    auto iter = redacted_elements.find(tag);
    if (iter != redacted_elements.end()){
        return iter->second;
    }
    for(auto &[etag,range] : elements){
        if(etag == tag) {
            auto &[start, end] = range;
            DicomElementView view(data, start);
            std::string value{std::string_view(view.GetValueHead(), view.value_length)};
            return value;
        }
    }
    return "";
}
