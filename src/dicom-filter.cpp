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

OrthancPluginDicomInstance* DicomFilter::GetFilteredInstance(){
    using globals::filter_list;
    char msg_buffer[256] = {0};
    const char* readable_buffer = (const char*)data;
    size_t preamble = 128;
    size_t prefix = 4;
    OrthancPluginLogWarning(globals::context, "Filter: Parsing received dicom data");
    // there is no valid header if the file size is smaller than 132 bytes
    if(size <= preamble + prefix){
        OrthancPluginLogError(globals::context, "DicomInstance does not have a valid size");
        return nullptr;
    }
    // the DICOM file header must end with DICM
    if(std::string_view(readable_buffer+preamble,prefix) != "DICM"){
        // apparently not a DICOM file... so...
        OrthancPluginLogError(globals::context, "DicomInstance does not match a valid DICOM format");
        return nullptr;
    }
    // parse dicom data elements
    bool do_nothing = true;
    std::vector<std::pair<size_t,size_t>> discard_list;
    for(size_t i = preamble+prefix; i < size;){
        // parse next element
        DicomElement element(readable_buffer,i);
        // print element info
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
        // get next element index
        size_t j = element.GetNextIndex();
        // track elements to be filtered
        if(filter_list.contains(element.tag)){
            sprintf(msg_buffer, "filtering tag: %s,%s", element.HexGroup().c_str(), element.HexElement().c_str());
            OrthancPluginLogWarning(globals::context, msg_buffer);
            discard_list.emplace_back(i, j);
            do_nothing = false;
        }
        i = j;
    }
    // return the original instance if nothing was queued for filtration
    if(do_nothing){
        // todo: figure out an elegant way to deal with this edge case (probably gonna involve not returning a ptr at all)
        OrthancPluginLogWarning(globals::context, "Filter: nothing to do");
        return (OrthancPluginDicomInstance*)original_instance;
    }
    // invert discard_list into keep_list
    size_t i = 0;
    std::vector<std::pair<size_t,size_t>> keep_list;
    for(auto pair : discard_list){
        keep_list.emplace_back(i,pair.first);
        i = pair.second;
    }
    keep_list.emplace_back(i,size);
    // calculate filtered buffer size
    size_t new_size = 0;
    for(auto pair : keep_list){
        new_size += pair.second - pair.first;
    }
    // compile filtered buffer
    i = 0;
    const char* buffer = new char[new_size];
    OrthancPluginLogWarning(globals::context, "Filter: compile new dicom buffer");
    for(auto pair : keep_list){
        size_t copy_size = pair.second-pair.first;
        memcpy((void*)(buffer+i), (void*)(readable_buffer+pair.first), copy_size);
        i += copy_size;
    }
    OrthancPluginLogWarning(globals::context, "Filter: create new dicom instance");
    // todo: save dicom instance, cause apparently CreateDicomInstance doesn't
    return OrthancPluginCreateDicomInstance(globals::context, buffer, new_size);
}