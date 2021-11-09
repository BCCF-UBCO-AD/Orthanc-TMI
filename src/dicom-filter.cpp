#include "dicom-filter.h"
#include "dicom-element.h"
#include <string>
#include <cstring>
#include <vector>

DicomFilter::DicomFilter(const OrthancPluginDicomInstance* readonly_instance){
    original_instance = readonly_instance;
    data = OrthancPluginGetInstanceData(globals::context, readonly_instance);
    size = OrthancPluginGetInstanceSize(globals::context, readonly_instance);
}

//std::string DecToHex(uint16_t value) {
//    std::stringstream stream;
//    stream << std::hex << value;
//    return std::string(stream.str());
//}

OrthancPluginDicomInstance* DicomFilter::GetFilteredInstance(){
    OrthancPluginLogError(globals::context, "Parse dicom data");
    using globals::filter_list;
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    if(size <= preamble + prefix){
        OrthancPluginLogError(globals::context, "DicomInstance does not have a valid size");
        return nullptr;
    }
    std::string test(std::string_view(readable_buffer+preamble,prefix));
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        OrthancPluginLogError(globals::context, "DicomInstance does not match a valid DICOM format");
        return nullptr;
    }
    // move the read head to where the tag data begins
    readable_buffer = readable_buffer+preamble+prefix;
    size_t remainder = size-(preamble+prefix);
    std::vector<std::pair<size_t,size_t>> keep_list;
    bool filtered = false;
    char msg_buffer[256] = {0};
    for(size_t i = 0; i < remainder;){
        DicomElement element(readable_buffer,i);
        sprintf(msg_buffer,"[%s] (%s,%s)->(%s)\n idx: %zu, next: %zu, size: %zu, bytes: %zu, length: %d",
                element.VR.c_str(),
                element.HexGroup().c_str(),
                element.HexElement().c_str(),
                element.HexTag().c_str(),
                element.idx,
                element.GetNextIndex(),
                element.size,
                element.bytes,
                (int)element.length);
        OrthancPluginLogInfo(globals::context, msg_buffer);
        size_t j = element.GetNextIndex();
        if(filter_list.contains(element.tag)){
            sprintf(msg_buffer, "filtering tag: %s,%s", element.HexGroup().c_str(), element.HexElement().c_str());
            OrthancPluginLogError(globals::context, msg_buffer);
            filtered = true;
        } else {
            keep_list.emplace_back(i, j);
        }
        i = j;
    }
    if(!filtered){
        // todo: figure out an elegant way to deal with this edge case (probably gonna involve not returning a ptr at all)
        OrthancPluginLogError(globals::context, "Filter: nothing to do");
        return (OrthancPluginDicomInstance*)original_instance;
    }
    size_t new_size = 0;
    for(auto pair : keep_list){
        new_size += pair.second - pair.first;
    }
    const char* buffer = new char[new_size];
    size_t buffer_head = 0;
    OrthancPluginLogError(globals::context, "Filter: compile new dicom");
    for(auto pair : keep_list){
        size_t copy_size = pair.second-pair.first;
        memcpy((void*)(buffer+buffer_head), (void*)(readable_buffer+pair.first), copy_size);
        buffer_head += copy_size;
    }
    // todo: test if CreateDicomInstance triggers a new filter callback
    OrthancPluginLogError(globals::context, "Filter: saving filtered dicom");
    return OrthancPluginCreateDicomInstance(globals::context, buffer, new_size);
}