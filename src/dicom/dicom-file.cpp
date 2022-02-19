#include <dicom-file.h>
#include <dicom-element-view.h>
#include <plugin-configure.h>
#include <vector>
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
        std::string key = HexToKey(DecToHex(element.tag,4));
        const char* kc = key.c_str();
        sprintf(msg_buffer,"[%s] (%s)->(%d)\n idx: %zu, next: %zu, size: %zu, value offset: %zu, length: %d",
                element.VR.c_str(),
                kc,
                element.tag,
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
    for(auto &[groupby,key] : PluginConfigurer::GetHardlinks()){
        auto tag = HexToDec(KeyToHex(key));
        try {
            // todo: implement GetData(uuid,tag) and the presumed SetData(uuid,tag,value) with an integration somewhere in DicomAnonymizer
            //hardlink_to("/"+groupby+"/", GetData(uuid,tag));
        } catch (const std::exception &e) {
            DEBUG_LOG(PLUGIN_ERRORS,"We failed to create a hard link. It may already exist.");
            std::cerr << e.what() << std::endl;
        }
    }
}