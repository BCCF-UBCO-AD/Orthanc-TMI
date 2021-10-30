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
        // todo: throw error, or something
    }
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        // todo: throw error, or something
    }
    // move the read head to where the tag data begins
    readable_buffer = readable_buffer+preamble+prefix;
    size_t remainder = size-(preamble+prefix);
    bool filtered = false;
    for(size_t i = 0; i < remainder; ++i){
        // todo: read tag, convert to uint32_t
        uint32_t tag = -1;
        if(filter_list.contains(tag)){
            filtered = true;
        }
        /* todo:
         *  construct filtered data buffer
         *  option 1: track the filtered tags, use tracking data to copy into a perfectly sized buffer
         *  option 2: copy data into 'size' sized buffer while reading, don't copy filtered tags
         * Option 1 is probably the way to go
         */
    }
    if(!filtered){
        return original_instance;
    }
    // todo: calculate size
    size_t new_size = size - 0;
    const char* buffer = new char[new_size];
    // todo: copy non-filtered data to new buffer
    return OrthancPluginCreateDicomInstance(globals::context, buffer, new_size);
}