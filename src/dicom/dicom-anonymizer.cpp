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
    for (tag_uint64_t tag: blacklist) {
        auto hex = DecToHex(tag, 4);
        std::cout << (hex.length() > 4 ? HexToKey(hex) : hex) << std::endl;
    }
    std::cout << "whitelist: " << std::endl;
    for (tag_uint64_t tag: whitelist) {
        auto hex = DecToHex(tag, 4);
        std::cout << (hex.length() > 4 ? HexToKey(hex) : hex) << std::endl;
    }
}

size_t DicomAnonymizer::BuildWork(const DicomFile &file) {
    static int count = 0;
    ++count;
    std::vector<Range> discard_list;
    // iterate the DICOM data elements (file.elements is in the same order as the binary file/data)
    for (size_t removed = 0; const auto &[tag, range]: file.elements) {
        const auto &[start, end] = range;
        DicomElementView view(file.data, start);
        // check what we need to do with this data element
        if (Filter(tag)) {
            // redact data
            discard_list.push_back(range);
            removed += range.second - range.first;
            old_data.emplace(tag, std::string_view(view.GetValueHead(), view.value_length));
        } else if (Truncate(view)) {
            // truncate date
            std::string date(std::string_view(view.GetValueHead(), view.value_length));
            if (!date.empty()) {
                auto original = date;
                date = TruncateDate(date, PluginConfigurer::GetDateFormat(tag));
                if (date != original) {
                    dates.emplace(view.GetValueIndex() - removed, date);
                    old_data.emplace(tag, original);
                }
            }
        }
    }
    // if containers are empty there is no work
    if (discard_list.empty() && dates.empty()) {
        return 0;
    }
    size_t size = file.size;
    // invert discard_list into keep_list
    if (!discard_list.empty()) {
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
    return size;
}

bool DicomAnonymizer::Anonymize(DicomFile &file) {
    static int count = 0;
    ++count;
    // if the file is invalid do nothing
    if (!file.IsValid()) {
        DEBUG_LOG(0, "Anonymize: received invalid DICOM file");
        return false;
    }
    // if the file has nothing to anonymize
    size_t size = BuildWork(file);
    if (size == 0) {
        DEBUG_LOG(0, "Anonymize: no changes");
        return true;
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
            void* src = ((char*) file.data) + start;
            std::memcpy(dst, src, copy_size);
            sprintf(msg, "i: %ld, range.1: %ld, range.2: %ld, copy_size: %ld", index, start, end, copy_size);
            DEBUG_LOG(DEBUG_2, msg);
            // update the new buffer's index (everything left of this index is the copied data so far)
            index += copy_size;
        }
    }
    // truncate dates
    for (const auto &[index, date]: dates) {
        // rewrite the appropriate part of the buffer
        void* dst = buffer.get() + index;
        std::memcpy(dst, date.c_str(), 8);
    }
    // check the filtered output is valid
    DicomFile filtered = DicomFile(buffer, size);
    if (filtered.IsValid()) {
        file = filtered;
        file.redacted_elements = std::move(old_data);
        return true;
    }
    DEBUG_LOG(0, "Anonymize: invalid DICOM output");
    return false;
}

int DicomAnonymizer::Configure(const nlohmann::json &config) {
    try {
        char msg_buffer[256] = {0};
        auto filter = config.at("DataAnon").at("Filter");
        auto date_truncation = config.at("DataAnon").at("DateTruncation");
        for (const auto &iter: filter["blacklist"]) {
            // get tag string, convert to decimal
            auto tag_key = iter.get<std::string>();
            if (tag_key.length() == 9) {
                // register tag
                tag_uint64_t tag = HexToDec(KeyToHex(tag_key));
                blacklist.emplace(tag);
                sprintf(msg_buffer, "DataAnon: blacklist registered tag: %s", tag_key.c_str());
                DEBUG_LOG(0, msg_buffer);
            } else if (tag_key.length() == 4) {
                // register group
                uint64_t group_code = HexToDec(tag_key);
                blacklist.emplace(group_code);
                sprintf(msg_buffer, "DataAnon: blacklist registered group: %s", tag_key.c_str());
                DEBUG_LOG(0, msg_buffer);
            } else {
                //bad format, we're gonna fail graciously and let the plugin keep moving
                DEBUG_LOG(PLUGIN_ERRORS, "invalid entry in Dicom-Filter blacklist (must be 4 or 8 hex-digits eg. '0017,0010')");
            }
        }
        for (const auto &iter: filter["whitelist"]) {
            // get tag string, convert to decimal
            auto tag_key = iter.get<std::string>();
            if (tag_key.length() == 9) {
                // register tag
                tag_uint64_t tag = HexToDec(KeyToHex(tag_key));
                whitelist.emplace(tag);
                sprintf(msg_buffer, "DataAnon: whitelist registered tag: %s", tag_key.c_str());
                DEBUG_LOG(0, msg_buffer);
            } else {
                //bad format, we're gonna fail graciously and let the plugin keep moving
                DEBUG_LOG(PLUGIN_ERRORS, "invalid entry in Dicom-Filter whitelist (must be 8 digits eg. '0017,0010')");
            }
        }
        for (auto &[tag_key, value]: date_truncation.items()) {
            // we want dates configured to truncate to not be accidentally discarded, so we add them to the whitelist
            // one of these isn't a tag_key but rather "default", so this is how we check
            if (tag_key.length() == 9 && tag_key.find(',') != std::string::npos) {
                whitelist.emplace(HexToDec(KeyToHex(tag_key)));
            }
        }
    } catch (const std::exception &e) {
        DEBUG_LOG(PLUGIN_ERRORS,"Failed to configure DicomAnonymizer. The json appears to be missing important sections.")
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
