#include <dicom-filter.h>
#include <dicom-element.h>
#include <dicom-tag.h>
#include <cstdint>
#include <iostream>

DicomFilter::DicomFilter(const nlm::json &config) {
    try {
        auto log = [](uint64_t &code) {
            char msg_buffer[256] = {0};
            sprintf(msg_buffer, "filter registered tag code: %ld", code);
            if(globals::context) OrthancPluginLogWarning(globals::context, msg_buffer);
        };// log the tags registered
        if(config.contains("Dicom-Filter")) {
            for (const auto &iter: config["Dicom-Filter"]["blacklist"]) {
                // get tag string, convert to decimal
                auto tag_entry = iter.get<std::string>();
                if (tag_entry.length() == 9) {
                    // register tag
                    tag_entry.append(tag_entry.substr(0, 4));
                    tag_entry.erase(0, 5);
                    uint64_t tag_code = HexToDec(tag_entry);
                    blacklist.emplace(tag_code);
                    log(tag_code);
                } else if (tag_entry.length() == 4) {
                    // register group
                    uint64_t group_code = HexToDec(tag_entry);
                    blacklist.emplace(group_code);
                    log(group_code);
                } else {
                    //bad format, we're gonna fail graciously and let the plugin keep moving
                    if (globals::context) OrthancPluginLogWarning(globals::context, "invalid entry in Dicom-Filter blacklist (must be 4 or 8 hex-digits eg. '0017,0010')");
                }
            }
            for (const auto &iter: config["Dicom-Filter"]["whitelist"]) {
                // get tag string, convert to decimal
                auto tag_entry = iter.get<std::string>();
                if (tag_entry.length() == 9) {
                    // register tag
                    tag_entry.append(tag_entry.substr(0, 4));
                    tag_entry.erase(0, 5);
                    uint64_t tag_code = HexToDec(tag_entry);
                    whitelist.emplace(tag_code);
                    log(tag_code);
                } else {
                    //bad format, we're gonna fail graciously and let the plugin keep moving
                    if (globals::context) OrthancPluginLogWarning(globals::context, "invalid entry in Dicom-Filter whitelist (must be a full tag ie. 'xxxx,xxxx')");
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        //todo: should we exit?
        //exit(1);
    }
}
DicomFilter::DicomFilter(const DicomFilter &other) {
    blacklist = other.blacklist;
    whitelist = other.whitelist;
    viPHI_list = other.viPHI_list;
}
DicomFilter DicomFilter::ParseConfig(const nlm::json &config) {
    return DicomFilter(config);
}
simple_buffer DicomFilter::ApplyFilter(DicomFile &file) {
    if(!ready) {
        DEBUG_LOG("ApplyFilter: !ready");
        ready = true;
        // todo: implement DOB truncation, and any other special edge cases
        if (file.is_valid) {
            std::vector<Range> discard_list;
            for (const auto &[tag_code,range]: file.elements) {
                const auto &[start,end] = range;
                if (!whitelist.contains(tag_code) && (blacklist.contains(tag_code) || blacklist.contains(tag_code&GROUP_MASK))) {
                    discard_list.push_back(range);
//                    if (viPHI_list.contains(tag_code)) {
//                        DicomElement e((const char*) file.data, start);
//                        // todo: PHI stuff?
//                        if (e.value_length != -1) {
//                            std::string value(std::string_view(e.GetValueHead(), e.value_length));
//                            discarded[tag_code] = value;
//                        }
//                    }
                }
            }
            if (discard_list.empty()) {
                DEBUG_LOG("ApplyFilter: discard list is empty");
                return std::make_tuple(nullptr,0);
            }
            // invert discard_list into keep_list
            size_t i = 0;
            std::vector<Range> keep_list;
            size_t new_size = file.size;
            for (const auto &[start,end]: discard_list) {
                assert(i < end);
                keep_list.emplace_back(i, start);
                new_size -= end - start;
                i = end; //why is i correct?
            }
            keep_list.emplace_back(i, file.size);
            // compile filtered buffer
            i = 0;
            std::unique_ptr<char[]> buffer(new char[new_size]);
            if(globals::context) OrthancPluginLogWarning(globals::context, "Filter: compile new dicom buffer");
            for (const auto &[start,end]: keep_list) {
                char msg[128] = {0};
                size_t copy_size = end - start;
                if(copy_size != 0) {
                    std::memcpy((void*)(buffer.get() + i),(void*)(((char*) file.data) + start), copy_size);
                    sprintf(msg, "i: %ld, range.1: %ld, range.2: %ld, copy_size: %ld", i, start, end, copy_size);
                    DEBUG_LOG(msg);
                    i += copy_size;
                }
            }
            DEBUG_LOG("ApplyFilter: new buffer complete");
            return std::make_tuple(std::move(buffer),new_size);
        }
    }
    DEBUG_LOG("ApplyFilter: ready");
    return std::make_tuple(nullptr,0);;
}
