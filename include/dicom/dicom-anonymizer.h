#pragma once
#include <dicom-file.h>
#include <dicom-element-view.h>

class DicomAnonymizer {
    friend class PluginConfigurer;
    friend class TestAnonymizer;

private:
    using idx = size_t;
    using tag_code = uint64_t;
    static std::unordered_set<tag_code> blacklist;
    static std::unordered_set<tag_code> whitelist;
    std::unordered_map<idx, std::string> dates;
    std::vector<Range> keep_list;
    DicomFile output;
protected:
    static int Configure(const nlm::json &config);
    static bool Filter(tag_code tag);
    static bool Truncate(DicomElementView &view);
    bool BuildWork(DicomFile &file);
    DicomAnonymizer() = default;
public:
    bool Anonymize(DicomFile &file);
};
