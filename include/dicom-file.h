#pragma once
#include <core.h>
#include <dicom-filter.h>
#include <unordered_map>

class DicomFile{
    using Range = std::pair<size_t,size_t>;
private:
    const OrthancPluginDicomInstance* instance = nullptr;
    const void* data;
    size_t size;
    std::unordered_map<uint64_t, Range> elements;
    bool is_valid = true;
protected:
    bool parse_file();
public:
    DicomFile(const OrthancPluginDicomInstance* instance);
    DicomFile(const void* data, size_t size);
    std::tuple<nlm::json,std::unique_ptr<char[]>,size_t> ApplyFilter(const DicomFilter &filter);
    bool IsValid() const { return is_valid; }
};
