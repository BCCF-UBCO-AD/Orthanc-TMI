#pragma once
#include <dicom-file.h>
#include <dicom-element-view.h>
#include <dicom-tag.h>

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
    static std::unordered_set<tag_code> blacklist;
    static std::unordered_set<tag_code> whitelist;
    std::unordered_map<idx, std::string> dates;
    std::vector<Range> keep_list;
protected:
    static int Configure(const nlm::json &config);
    static bool Filter(tag_code tag);
    static bool Truncate(DicomElementView &view);
    size_t BuildWork(const DicomFile &file);
    DicomAnonymizer() = default;
public:
    bool Anonymize(DicomFile &file);
    static void debug();
};
