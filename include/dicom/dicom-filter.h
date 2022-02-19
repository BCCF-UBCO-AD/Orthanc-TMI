#pragma once
#include <core.h>
#include <dicom-file.h>

// an alias for a smart pointer buffer and its allocation size
using simple_buffer = std::tuple<std::unique_ptr<char[]>, size_t>;

/* class: DicomFilter
 *  Used to filter data from a dicom file.
 *
 *  This class will be configured via json to have particular sets of data to remove, keep, and potentially both.
 *
 *  Provides methods to:
 *   - configure the filter
 *   - apply the filter
 *   - reset the filter object's internal state
 */
class DicomFilter{
private:
    std::unordered_set<uint64_t> blacklist;
    std::unordered_set<uint64_t> whitelist; //todo: rename
    std::unordered_set<uint64_t> viPHI_list; //todo: rename
    nlm::json discarded;
    bool ready = false;
protected:
    DicomFilter(const nlm::json &config);
public:
    DicomFilter(const DicomFilter &other);
    static DicomFilter ParseConfig(const nlm::json &config);
    simple_buffer ApplyFilter(DicomFile &file);
    void reset() {
        ready = false;
        discarded = {};
    }
};
