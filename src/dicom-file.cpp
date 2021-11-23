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
        elements.emplace(element.tag, std::make_pair(i,j));
        i = j;
    }
    is_valid = i == size;
    return is_valid;
}

// todo: flip relationship around (ie. DicomFilter::filter(DicomFile))
std::tuple<nlm::json,std::unique_ptr<char[]>,size_t> DicomFile::ApplyFilter(const DicomFilter &filter) {
    // todo: implement DOB truncation, and any other special edge cases
    size_t new_size = 0;
    std::unique_ptr<char[]> buffer = nullptr;
    nlm::json discarded;
    if(is_valid) {
        std::vector<Range> discard_list;
        for(auto element_info : elements) {
            uint64_t tag_code = element_info.first;
            if (filter.FilterTag(tag_code)) {
                const auto &range = element_info.second;
                discard_list.push_back(range);
                if (filter.KeepData(tag_code)) {
                    DicomElement e((const char*)data, range.first);
                    // todo: PHI stuff?
                    if(e.value_length != -1) {
                        std::string value(std::string_view(e.GetValueHead(), e.value_length));
                        discarded[tag_code] = value;
                    }
                }
            }
        }
        if (discard_list.empty()) {
            return std::make_tuple(discarded,std::move(buffer),new_size);
        }
        // invert discard_list into keep_list
        size_t i = 0;
        std::vector<Range> keep_list;
        for (auto pair: discard_list) {
            keep_list.emplace_back(i, pair.first);
            i = pair.second;
            new_size += pair.first - i;
        }
        keep_list.emplace_back(i, size);
        new_size += size - i;
        // compile filtered buffer
        i = 0;
        std::unique_ptr<char[]> buffer(new char[new_size]);
        OrthancPluginLogWarning(globals::context, "Filter: compile new dicom buffer");
        for (auto pair: keep_list) {
            size_t copy_size = pair.second - pair.first;
            memcpy((void *) (buffer.get() + i), (void *) ((char *) data + pair.first), copy_size);
            i += copy_size;
        }
    }
    return std::make_tuple(discarded,std::move(buffer),new_size);
}
