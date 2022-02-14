#pragma once
#include <core.h>
//#include <dicom-filter.h>
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
    using tag = uint64_t;
private:
    std::unique_ptr<char[]> buffer;
    const OrthancPluginDicomInstance* instance = nullptr;
    const void* data = nullptr;
    size_t size = 0;

    std::vector<std::tuple<tag, Range>> elements;
    bool is_valid = true;
    bool is_const = true;
protected:
    explicit DicomFile(size_t size); // Allocate a buffer and create an empty DicomFile
    void MakeHardlinks(const fs::path &master_path);
public:
    explicit DicomFile(const OrthancPluginDicomInstance* instance);
    DicomFile(void* data, size_t size);
    DicomFile(const void* data, size_t size);
    DicomFile()= default;
    // todo: this should be tested
    DicomFile& operator=(DicomFile &&other) noexcept;

    static bool Parse(void* data, size_t size);
    bool Parse();
    bool IsValid() const { return is_valid; }
    OrthancPluginErrorCode Write(const fs::path &master_path);
};