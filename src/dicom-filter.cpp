#include "dicom-filter.h"
#include <string>
#incldue <cstring>
#include <sstream>
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

const OrthancPluginDicomInstance* DicomFilter::GetFilteredInstance(){
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
    for(size_t i = 0; i < remainder; ++i){
        size_t bytes = 6;
        uint32_t tag = *(uint32_t*)(readable_buffer+i);
        std::string VR(std::string_view(readable_buffer+i+4,2));
        uint32_t value_length;
        if(VR == "OB" || VR == "OW" || VR == "SQ" || VR == "UN"){
            // VR's which indicate a type 1 header (https://www.leadtools.com/help/sdk/v21/dicom/api/overview-data-element-structure.html)
            value_length = *(uint32_t*)(readable_buffer+i+8); // 4 bytes
            bytes += 6;
        } else {
            // all other non-private data elements
            value_length = *(uint16_t*)(readable_buffer+i+6); // 2 bytes
            bytes += 2;
        }
        size_t length = bytes + value_length;
        if(filter_list.contains(tag)){
            filtered = true;
        } else {
            keep_list.emplace_back(i, i + length);
        }
        i += length;
    }
    if(!filtered){
        return original_instance;
    }
    size_t new_size = 0;
    for(auto pair : keep_list){
        new_size += pair.second - pair.first;
    }
    const char* buffer = new char[new_size];
    size_t buffer_head = 0;
    for(auto pair : keep_list){
        size_t copy_size = pair.second-pair.first;
        memcpy((void*)(buffer+buffer_head), (void*)(readable_buffer+pair.first), copy_size);
        buffer_head += copy_size;
    }
    // todo: test if CreateDicomInstance triggers a new filter callback
    return OrthancPluginCreateDicomInstance(globals::context, buffer, new_size);
}