#pragma once
#include <core.h>

class DicomFilter{
private:
    std::unordered_set<uint64_t> filter_list;
    std::unordered_set<uint64_t> viPHI_list; //todo: rename
protected:
    DicomFilter(const nlm::json &config);
public:
    static DicomFilter ParseConfig(const nlm::json &config);
    bool FilterTag(const uint64_t &tag_code) const;
    bool KeepData(const uint64_t &tag_code) const;
};
