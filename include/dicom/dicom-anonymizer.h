#pragma once
#include <core.h>
#include <json.h>
#include <dicom-file.h>
#include <dicom-element-view.h>
#include <dicom-tag.h>

#include <unordered_set>
#include <unordered_map>
#include <vector>

/* class: DicomAnonymizer
 *  Used to anonymize DICOM files.
 *
 *  The user first acquires the DICOM file data then creates a DicomFile with the data buffer.
 *  That DicomFile is then passed to the Anonymizer.
 *
 *  This could be used to make a standalone anonymization utility if someone were keen to. Wouldn't take much.
 *
 *  Provides methods to:
 *   - Anonymize a DicomFile
 *   - print the Anonymizer's configuration to stdout
 */
class DicomAnonymizer {
    friend class PluginConfigurer;
    friend class TestAnonymizer;

private:
    using idx = size_t;
    static std::unordered_set<tag_uint64_t> blacklist;
    static std::unordered_set<tag_uint64_t> whitelist;
    std::unordered_map<idx, std::string> dates;
    std::vector<Range> keep_list;
    std::unordered_map<tag_uint64_t, std::string> old_data;
protected:
    static int Configure(const nlm::json &config);
    static bool Filter(tag_uint64_t tag);
    static bool Truncate(DicomElementView &view);
    size_t BuildWork(const DicomFile &file);
public:
    bool Anonymize(DicomFile &file);
    static void debug();
};
