#include "dicom-filter.h"
#include <string>

DicomFilter::DicomFilter(const OrthancPluginDicomInstance* readonly_instance){
    original_instance = readonly_instance;
    data = OrthancPluginGetInstanceData(globals::context, readonly_instance);
    size = OrthancPluginGetInstanceSize(globals::context, readonly_instance);
}

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
        if(filter_list.contains(tag)){
            filtered = true;
            // todo: implement (track skipped data sections)
        }
        i += bytes + value_length;
    }
    /* todo:
     *  construct filtered data buffer
     *  option 1: track the filtered tags, use tracking data to copy into a perfectly sized buffer
     *  option 2: copy data into 'size' sized buffer while reading, don't copy filtered tags
     * Option 1 is probably the way to go
     */
    if(!filtered){
        return original_instance;
    }
    // todo: calculate size
    size_t new_size = size - 0;
    const char* buffer = new char[new_size];
    // todo: copy non-filtered data to new buffer
    // todo: test if CreateDicomInstance triggers a new filter callback
    return OrthancPluginCreateDicomInstance(globals::context, buffer, new_size);
}