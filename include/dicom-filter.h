#pragma once
#include <core.h>
#include <dicom-file.h>

using simple_buffer = std::tuple<std::unique_ptr<char[]>, size_t>;
class DicomFilter{
private:
    std::unordered_set<uint64_t> filter_list;
    std::unordered_set<uint64_t> viPHI_list; //todo: rename
    nlm::json discarded;
    bool ready = false;
protected:
    DicomFilter(const nlm::json &config);
public:
    static DicomFilter ParseConfig(const nlm::json &config);
    simple_buffer ApplyFilter(DicomFile &file);
    bool FilterTag(const uint64_t &tag_code) const;
    bool KeepData(const uint64_t &tag_code) const;
};
