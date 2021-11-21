#include <dicom-filter.h>
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
DicomFilter DicomFilter::ParseConfig(const nlm::json &config) {
    return DicomFilter(config);
}
bool DicomFilter::FilterTag(const uint64_t &tag_code) const {
    return filter_list.contains(tag_code) || filter_list.contains(tag_code&GROUP_MASK);
}
bool DicomFilter::KeepData(const uint64_t &tag_code) const {
    return viPHI_list.contains(tag_code) || viPHI_list.contains(tag_code&GROUP_MASK);
}
