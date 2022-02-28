#pragma once
#include <core.h>
#include <dicom-tag.h>
#include <unordered_map>

// an alias for a range as a std::pair
using Range = std::pair<size_t,size_t>;

/* class: DicomFile
 *  Used to parse and index a dicom file.
 *
 *  Takes a dicom instance and gets a buffer or takes just a raw buffer.
 *  With the buffer it uses DicomElementView to parse and iterate through the data.
 *  Records tags and their range in the buffer during the parsing.
 *  If the parsing results in an index equal to the length of the file then the data was parsed as valid.
 *
 *  Provides methods to:
 *   - check if the parsing was valid
 *   - to write to the dicom file to the default location (this may come in handy writing an update function to re-filter existing files with new specifications)
 */
class DicomFile{
    friend class DicomAnonymizer;
    friend class TestAnonymizer;
private:
    std::shared_ptr<char[]> buffer;
    const void* data = nullptr;
    size_t size = 0;

    std::vector<std::tuple<tag_uint64_t, Range>> elements;
    std::unordered_map<tag_uint64_t, std::string> redacted_elements;
    bool is_valid = true;
protected:
    void MakeHardlinks(const fs::path &master_path);
    std::string GetData(tag_uint64_t tag);
public:
    DicomFile(std::shared_ptr<char[]> &buffer, size_t size);
    DicomFile(const void* data, size_t size);
    DicomFile()= default;

    static bool Parse(void* data, size_t size);
    bool Parse();
    bool IsValid() const { return is_valid; }
    OrthancPluginErrorCode Write(const fs::path &master_path);
};
