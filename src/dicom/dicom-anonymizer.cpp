#include <dicom-anonymizer.h>
#include <dicom-tag.h>
#include <plugin-configure.h>
#include <date-truncation.h>
#include <iostream>

std::unordered_set<uint64_t> DicomAnonymizer::blacklist;
std::unordered_set<uint64_t> DicomAnonymizer::whitelist;

bool DicomAnonymizer::Filter(uint64_t tag) {
    return !whitelist.contains(tag) && (blacklist.contains(tag) || blacklist.contains(tag & GROUP_MASK));
}

bool DicomAnonymizer::Truncate(DicomElementView &view) {
    return view.VR == "DA" && view.value_length != 0;
}

void DicomAnonymizer::debug() {
    std::cout << "blacklist: " << std::endl;
    for(uint64_t tag_code : blacklist){
        auto hex = DecToHex(tag_code,4);
        std::cout << (hex.length() > 4 ? HexToKey(hex) : hex) << std::endl;
    }
    std::cout << "whitelist: " << std::endl;
    for(uint64_t tag_code : whitelist){
        auto hex = DecToHex(tag_code,4);
        std::cout << (hex.length() > 4 ? HexToKey(hex) : hex) << std::endl;
    }
}

bool DicomAnonymizer::BuildWork(const DicomFile &file) {
    static int count = 0;
    ++count;
    std::vector<Range> discard_list;
    // iterate the DICOM data elements (file.elements is in the same order as the binary file/data)
    for (const auto &[tag_code, range]: file.elements) {
        const auto &[start, end] = range;
        // check what we need to do with this data element
        std::string key = HexToKey(DecToHex(tag_code,4));
        const char* kc = key.c_str();
        if (Filter(tag_code)) {
            // redact data
            discard_list.push_back(range);
        } else if (DicomElementView view(file.data, start); Truncate(view)) {
            // truncate date
            std::string date(std::string_view(view.GetValueHead(), view.value_length));
            if (!date.empty()) {
                auto df = PluginConfigurer::GetDateFormat(tag_code);
                auto dfc = df.c_str();
                auto date0 = date;
                auto date0c = date0.c_str();
                date = TruncateDate(date, dfc);
                auto date2c = date.c_str();
                dates.emplace(view.GetValueIndex(), date);
            }
        }
    }
    // if containers are empty there is no work
    if (discard_list.empty() && dates.empty()) {
        return false;
    }

    size = file.size;
    // invert discard_list into keep_list
    if(!discard_list.empty()) {
        for (size_t last_index = 0, i = 0; const auto &[start, end]: discard_list) {
            keep_list.emplace_back(last_index, start);
            size -= end - start;  //the range's length can be subtracted since it is to be discarded
            last_index = end; //this will progressively increase, never decrease (because file.elements is in order)
            // if this is the final iteration
            if (++i == discard_list.size()) {
                // We need to ensure that we reach the end of the file.
                // Only if the last element is discarded this won't matter (ie. last_index == file.size)
                // adding it to the keep list also won't matter though, as the copy size will be 0 in that case
                keep_list.emplace_back(last_index, file.size);
            }
        }
    } else {
        // the discard list might be empty ie. keep everything
        keep_list.emplace_back(0, file.size);
    }
    return true;
}

DicomFile DicomAnonymizer::Anonymize(const DicomFile &file) {
    static int count = 0;
    size = 0;
    ++count;
    // if the file is invalid do nothing
    if (!file.IsValid()) {
        DEBUG_LOG(0, "Anonymize: received invalid DICOM file");
        return file;
    }
    // if the file has nothing to anonymize
    keep_list.clear();
    dates.clear();
    if (!BuildWork(file)) {
        DEBUG_LOG(0,"Anonymize: no changes");
        return file;
    }
    // Allocate new buffer
    std::shared_ptr<char[]> buffer = std::shared_ptr<char[]>(new char[size]);

    // begin anonymization
    // copy everything except filtered data
    for (size_t index = 0; const auto &[start, end]: keep_list) {
        char msg[128] = {0};
        size_t copy_size = end - start;
        if (copy_size != 0) {
            // copy from file.data at wherever the loop tells us to the new buffer at the current index in it
            void* dst = buffer.get() + index;
            void* src = ((char*)file.data) + start;
            std::memcpy(dst, src, copy_size);
            sprintf(msg, "i: %ld, range.1: %ld, range.2: %ld, copy_size: %ld", index, start, end, copy_size);
            DEBUG_LOG(1, msg);
            // update the new buffer's index (everything left of this index is the copied data so far)
            index += copy_size;
        }
    }
    // truncate dates
    for (const auto &[index, date]: dates) {
        // rewrite the appropriate part of the buffer
        void* dst = buffer.get() + index;
        std::memcpy(dst, date.c_str(), date.length());
    }
    // check the filtered output is valid
    DicomFile R = DicomFile(buffer,size);
    if (file.IsValid()) {
        return R;
    }
    DEBUG_LOG(0, "Anonymize: invalid DICOM output");
    return file;
}

int DicomAnonymizer::Configure(const nlohmann::json &config) {
    try {
        char msg_buffer[256] = {0};
        if(config.contains("Dicom-Filter")) {
            for (const auto &iter: config["Dicom-Filter"]["blacklist"]) {
                // get tag string, convert to decimal
                auto tag_entry = iter.get<std::string>();
                if (tag_entry.length() == 9) {
                    // register tag
                    uint64_t tag_code = HexToDec(KeyToHex(tag_entry));
                    blacklist.emplace(tag_code);
                    sprintf(msg_buffer, "filter registered tag code: %ld", tag_code);
                    DEBUG_LOG(0,msg_buffer);
                } else if (tag_entry.length() == 4) {
                    // register group
                    uint64_t group_code = HexToDec(tag_entry);
                    blacklist.emplace(group_code);
                    sprintf(msg_buffer, "filter registered tag code: %ld", group_code);
                    DEBUG_LOG(0,msg_buffer);
                } else {
                    //bad format, we're gonna fail graciously and let the plugin keep moving
                    DEBUG_LOG(PLUGIN_ERRORS,"invalid entry in Dicom-Filter blacklist (must be 4 or 8 hex-digits eg. '0017,0010')");
                }
            }
            for (const auto &iter: config["Dicom-Filter"]["whitelist"]) {
                // get tag string, convert to decimal
                auto tag_entry = iter.get<std::string>();
                if (tag_entry.length() == 9) {
                    // register tag
                    uint64_t tag_code = HexToDec(KeyToHex(tag_entry));
                    whitelist.emplace(tag_code);
                    sprintf(msg_buffer, "filter registered tag code: %ld", tag_code);
                    DEBUG_LOG(0,msg_buffer);
                } else {
                    //bad format, we're gonna fail graciously and let the plugin keep moving
                    DEBUG_LOG(PLUGIN_ERRORS,"invalid entry in Dicom-Filter whitelist (must be 8 digits eg. '0017,0010')");
                }
            }
            for (auto &[key,value] : config["Dicom-DateTruncation"].items()) {
                // we want dates configured to truncate to not be accidentally discarded, so we add them to the whitelist
                if (key.length() == 9 && key.find(',') != std::string::npos) {
                    whitelist.emplace(HexToDec(KeyToHex(value)));
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        //todo: should we exit?
        //exit(1);
        return -1;
    }
    return 0;
}
