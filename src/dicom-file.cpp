#include <dicom-file.h>
#include <dicom-element.h>
#include <vector>

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
    if(globals::context) OrthancPluginLogWarning(globals::context, "Filter: Parsing dicom data");
    // there is no valid header if the file size is smaller than 132 bytes
    if(size <= preamble + prefix){
        if(globals::context) OrthancPluginLogError(globals::context, "DicomFile does not have a valid size");
        is_valid = false;
        return false;
    }
    // the DICOM file header must end with DICM
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        if(globals::context) OrthancPluginLogError(globals::context, "DicomFile does not match a valid DICOM format");
        is_valid = false;
        return false;
    }
    // parse dicom data elements
    size_t i;
    for(i = preamble+prefix; i < size;){
        // parse next element
        DicomElement element(readable_buffer,i);
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
        if(globals::context) OrthancPluginLogInfo(globals::context, msg_buffer);
        // save element range
        size_t j = element.GetNextIndex();
        elements.emplace_back(element.tag, std::make_pair(i,j));
        i = j;
    }
    is_valid = i == size;
    return is_valid;
}