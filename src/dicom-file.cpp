#include <dicom-file.h>
#include <dicom-element-view.h>
#include <vector>
#include <fstream>

DicomFile::DicomFile(const OrthancPluginDicomInstance *instance) {
    this->instance = instance;
    data = OrthancPluginGetInstanceData(globals::context, instance);
    size = OrthancPluginGetInstanceSize(globals::context, instance);
    parse_file();
}

DicomFile::DicomFile(const void *data, size_t size) {
    this->data = data;
    this->size = size;
    parse_file();
}

bool DicomFile::parse_file() {
    char msg_buffer[256] = {0};
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    DEBUG_LOG("DicomFile: parsing dicom data");
    // there is no valid header if the file size is smaller than 132 bytes
    if(size <= preamble + prefix){
        if(globals::context) OrthancPluginLogError(globals::context, "DicomFile does not have a valid size, cannot continue parsing");
        is_valid = false;
        return false;
    }
    // the DICOM file header must end with DICM
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        if(globals::context) OrthancPluginLogError(globals::context, "DicomFile does not match a valid DICOM format, cannot continue parsing");
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
        DEBUG_LOG(msg_buffer);
        // save element range
        size_t j = element.GetNextIndex();
        // todo: should it be j-1?
        elements.emplace_back(element.tag, std::make_pair(i,j));
        i = j;
    }
    sprintf(msg_buffer,"buffer size = %ld, read head idx = %ld", size, i);
    DEBUG_LOG(msg_buffer);
    is_valid = i == size;
    return is_valid;
}

extern const fs::path GetPath(OrthancPluginContentType type, const char* uuid);
void DicomFile::Write(const char* uuid) {
    char msg[256] = {0};
    fs::path master_path = GetPath(OrthancPluginContentType_Dicom, uuid);
    fs::create_directories(master_path);
    sprintf(msg, "DicomFile: writing to %s", master_path.c_str());
    DEBUG_LOG(msg);
    std::fstream file(master_path, std::ios::binary | std::ios::out);
    file.write((const char*)data,size);
    file.close();
    DEBUG_LOG("DicomFile: write complete");
}