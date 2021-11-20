#pragma once
#include <core.h>
#include <unordered_map>

class DicomFile{
    using Range = std::pair<size_t,size_t>;
private:
    const OrthancPluginDicomInstance* instance = nullptr;
    const void* data;
    uint64_t size;
    std::unordered_map<uint32_t, Range> elements;
    bool is_valid = true;
protected:
    void parse_file();
public:
    DicomFile(){}
    DicomFile(const OrthancPluginDicomInstance* instance);
    DicomFile(const void* data, size_t size);
    std::tuple<std::unique_ptr<char[]>,size_t> ApplyFilter(TagFilter filter);
    bool IsValid() const { return is_valid; }
};
