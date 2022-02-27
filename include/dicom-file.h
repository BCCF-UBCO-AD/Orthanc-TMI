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
    friend class DicomFilter;
private:
    using tag = uint64_t;
    const OrthancPluginDicomInstance* instance = nullptr;
    const void* data;
    size_t size;
    std::vector<std::tuple<tag, Range>> elements;
    bool is_valid = true;
protected:
    bool parse_file();
public:
    DicomFile(const OrthancPluginDicomInstance* instance);
    DicomFile(const void* data, size_t size);
    //std::tuple<nlm::json,std::unique_ptr<char[]>,size_t> ApplyFilter(const DicomFilter &filter);
    bool IsValid() const { return is_valid; }
    void Write(const char* uuid);
    const void* GetData() {return data; }
};
