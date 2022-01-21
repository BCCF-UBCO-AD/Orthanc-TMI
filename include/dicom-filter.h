#pragma once
#include <core.h>
#include <dicom-file.h>

using simple_buffer = std::tuple<std::unique_ptr<char[]>, size_t>;
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
