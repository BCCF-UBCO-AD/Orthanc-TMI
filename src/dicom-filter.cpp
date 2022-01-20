#include <dicom-filter.h>
#include <dicom-element-view.h>
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
    /* To filter data from a dicom file we need an index of the data, which we have in file.elements
     * 1. iterate the index, and record data that matches the filter
     * 2. invert the ranges from what we recorded (we record what to discard, so this step creates information on what to keep)
     * 3. copy the data we need to keep into a new buffer and return
     */
    // todo: probably should make this class stateless
    if(!ready) {
        DEBUG_LOG("ApplyFilter: !ready");
        ready = true;
        if (file.is_valid) {
            std::vector<Range> discard_list;
            std::vector<Range> keep_list;
            // iterate the DICOM data elements (file.elements is in the same order as the binary file/data)
            for (const auto &[tag_code,range]: file.elements) {
                const auto &[start,end] = range;
                // check what we need to do with this data element
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
            // if the discard list is empty then there was nothing to filter
            if (discard_list.empty()) {
                DEBUG_LOG("ApplyFilter: discard list is empty");
                return std::make_tuple(nullptr,0);
            }
            size_t filtered_buffer_size = file.size;
            // invert discard_list into keep_list
            for (size_t last_index = 0,i = 0; const auto &[start,end]: discard_list) {
                keep_list.emplace_back(last_index, start);
                filtered_buffer_size -= end - start;  //the range's length can be subtracted since it is to be discarded
                last_index = end; //this will progressively increase, never decrease (because file.elements is in order)
                // if this is the final iteration
                if(++i == discard_list.size()){
                    // We need to ensure that we reach the end of the file.
                    // Only if the last element is discarded this won't matter (ie. last_index == file.size)
                    // adding it to the keep list also won't matter though, as the copy size will be 0 in that case
                    keep_list.emplace_back(last_index, file.size);
                }
            }

            // compile filtered buffer
            std::unique_ptr<char[]> buffer(new char[filtered_buffer_size]);
            if(globals::context) OrthancPluginLogWarning(globals::context, "Filter: compile new dicom buffer");
            for (size_t index = 0; const auto &[start,end]: keep_list) {
                char msg[128] = {0};
                size_t copy_size = end - start;
                if(copy_size != 0) {
                    // copy from file.data at wherever the loop tells us to the new buffer at the current index in it
                    std::memcpy((void*)(buffer.get() + index), (void*)(((char*) file.data) + start), copy_size);
                    sprintf(msg, "i: %ld, range.1: %ld, range.2: %ld, copy_size: %ld", index, start, end, copy_size);
                    DEBUG_LOG(msg);
                    // update the new buffer's index (everything in front of this index is the copied data so far)
                    index += copy_size;
                }
            }
            DEBUG_LOG("ApplyFilter: new buffer complete");
            return std::make_tuple(std::move(buffer), filtered_buffer_size);
        }
    }
    DEBUG_LOG("ApplyFilter: ready");
    return std::make_tuple(nullptr,0);;
}
