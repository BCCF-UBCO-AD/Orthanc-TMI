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

bool DicomAnonymizer::BuildWork(DicomFile &file) {
    std::vector<Range> discard_list;
    // iterate the DICOM data elements (file.elements is in the same order as the binary file/data)
    for (const auto &[tag_code, range]: file.elements) {
        const auto &[start, end] = range;
        // check what we need to do with this data element
        if (Filter(tag_code)) {
            // redact data
            discard_list.push_back(range);
        } else if (DicomElementView view(file.data, start); Truncate(view)) {
            // truncate date
            std::string date(std::string_view(view.GetValueHead(), view.value_length));
            date = TruncateDate(date, PluginConfigurer::GetDateFormat(tag_code).c_str());
            dates.emplace(view.GetValueIndex(), date);
        }
    }
    // if containers are empty there is no work
    if (discard_list.empty() && dates.empty()) {
        return false;
    }

    size_t filtered_buffer_size = file.size;
    // invert discard_list into keep_list
    if(!discard_list.empty()) {
        for (size_t last_index = 0, i = 0; const auto &[start, end]: discard_list) {
            keep_list.emplace_back(last_index, start);
            filtered_buffer_size -= end - start;  //the range's length can be subtracted since it is to be discarded
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
    // Allocate new file
    output = DicomFile(filtered_buffer_size);
    return true;
}

bool DicomAnonymizer::Anonymize(DicomFile &file) {
    // if we received a valid file
    if (file.IsValid()) {
        // if the file has nothing to anonymize
        if (!BuildWork(file)) {
            DEBUG_LOG(0,"Anonymize: no changes");
            return true;
        }
        // begin anonymization
        // copy everything except filtered data
        for (size_t index = 0; const auto &[start, end]: keep_list) {
            char msg[128] = {0};
            size_t copy_size = end - start;
            if (copy_size != 0) {
                // copy from file.data at wherever the loop tells us to the new buffer at the current index in it
                std::memcpy(output.buffer.get() + index, (char*) file.data + start, copy_size);
                sprintf(msg, "i: %ld, range.1: %ld, range.2: %ld, copy_size: %ld", index, start, end, copy_size);
                DEBUG_LOG(1, msg);
                // update the new buffer's index (everything left of this index is the copied data so far)
                index += copy_size;
            }
        }
        // truncate dates
        for (const auto &[index, date]: dates) {
            // rewrite the appropriate part of the buffer
            std::memcpy(output.buffer.get() + index, date.c_str(), date.length());
        }
        // check the filtered output is valid
        output.Parse();
        if (output.IsValid()) {
            file = std::move(output);
            return true;
        }
    }
    DEBUG_LOG(0,"Anonymize: received invalid DICOM file");
    return false;
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
            for (auto &[key,value] : config["Dicom-TruncateDate"].items()) {
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
