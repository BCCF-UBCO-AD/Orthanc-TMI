#include <dicom-filter.h>
#include <dicom-element.h>
#include <dicom-tag.h>
#include <cstdint>

DicomFilter::DicomFilter(const nlm::json &config) {
    auto log = [](uint64_t &code){
        char msg_buffer[256] = {0};
        sprintf(msg_buffer, "filter registered tag code: %d", code);
        OrthancPluginLogWarning(globals::context, msg_buffer);
    };// log the tags registered
    for (const auto &iter: config["Dicom-Filter"]["tags"]) {
        // todo: check that the string has 9 characters; true: do below, false: implement full group filters (eg. "0002,*", "0002")
        // get tag string, convert to decimal
        auto tag_entry = iter.get<std::string>();
        if(tag_entry.length() == 9){
            // register tag
            tag_entry.append(tag_entry.substr(0, 4));
            tag_entry.erase(0, 5);
            uint64_t tag_code = HexToDec(tag_entry);
            filter_list.emplace(tag_code);
            log(tag_code);
        } else if (tag_entry.length() == 4) {
            // register group
            uint64_t group_code = HexToDec(tag_entry);
            filter_list.emplace(group_code);
            log(group_code);
        } else {
            //bad format, we're gonna fail graciously and let the plugin keep moving
            OrthancPluginLogWarning(globals::context,"invalid entry in Dicom-Filter tags");
        }
    }
}
DicomFilter::DicomFilter(const DicomFilter &other) {
    filter_list = other.filter_list;
    viPHI_list = other.viPHI_list;
}
DicomFilter DicomFilter::ParseConfig(const nlm::json &config) {
    return DicomFilter(config);
}
simple_buffer DicomFilter::ApplyFilter(DicomFile &file) {
    if(!ready) {
        ready = true;
        // todo: implement DOB truncation, and any other special edge cases
        size_t new_size = 0;
        std::unique_ptr<char[]> buffer = nullptr;
        if (file.is_valid) {
            std::vector<Range> discard_list;
            for (auto element_info: file.elements) {
                uint64_t tag_code = element_info.first;
                if (filter_list.contains(tag_code)) {
                    const auto &range = element_info.second;
                    discard_list.push_back(range);
                    if (viPHI_list.contains(tag_code)) {
                        DicomElement e((const char*) file.data, range.first);
                        // todo: PHI stuff?
                        if (e.value_length != -1) {
                            std::string value(std::string_view(e.GetValueHead(), e.value_length));
                            discarded[tag_code] = value;
                        }
                    }
                }
            }
            if (discard_list.empty()) {
                std::make_tuple(nullptr,0);
            }
            // invert discard_list into keep_list
            size_t i = 0;
            std::vector<Range> keep_list;
            for (auto pair: discard_list) {
                keep_list.emplace_back(i, pair.first);
                i = pair.second;
                new_size += pair.first - i;
            }
            keep_list.emplace_back(i, file.size);
            new_size += file.size - i;
            // compile filtered buffer
            i = 0;
            std::unique_ptr<char[]> buffer(new char[new_size]);
            OrthancPluginLogWarning(globals::context, "Filter: compile new dicom buffer");
            for (auto pair: keep_list) {
                size_t copy_size = pair.second - pair.first;
                memcpy(buffer.get() + i, ((char*) file.data) + pair.first, copy_size);
                i += copy_size;
            }
            return std::make_tuple(std::move(buffer),new_size);
        }
    }
    return std::make_tuple(nullptr,0);;
}
