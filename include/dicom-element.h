#pragma once
#include <core.h>
#include <dicom-tag.h>

/* class: DicomElement
 *  Takes an index and data buffer to a DICOM file.
 *  Using the buffer and index, it locates DICOM data element information.
 *  This information locates the next DICOM data element index in the buffer.
 *
 *  Provides methods to:
 *   - calculate the next element's index
 *   - get hex for group id
 *   - get hex for element id
 *   - get hex for entire tag (group+element)
 */
class DicomElement {
    // below DicomElement locates and aliases data within the dicom's data buffer, then calculates appropriate length/size values
public:
    const char *const buffer;
    const char *const hex_buffer;
    const uint64_t idx;
    const uint32_t &tag = *(uint32_t *) (buffer + idx);
    const uint16_t &group = *(uint16_t *) (buffer + idx);
    const uint16_t &element = *(uint16_t *) (buffer + idx + 2);;
    const std::string VR = std::string(std::string_view(buffer + idx + 4, 2));
    const uint32_t length = CalcLength();
    const uint64_t size = CalcSize();
    size_t bytes;

    DicomElement(const char *buffer, uint64_t index, const char *hex_buffer = nullptr)
            : buffer(buffer),
              hex_buffer(hex_buffer),
              idx(index) {
// This preprocessor allows printing DataElement debug information as part of construction
#ifdef DEBUG_
        uint32_t len = length == -1 ? 0 : length;
        printf("[%s](%s,%s)->(%s)\n",
               VR.c_str(),
               HexGroup().c_str(),
               HexElement().c_str(),
               HexTag().c_str());
        printf(" idx: %zu, next: %zu, size: %zu, bytes: %zu, length: %d\n",
               idx,GetNextIndex(),size,bytes,(int)length);
        std::string value;

        if(VR == "UL") {
            value = std::to_string(*(uint32_t*)(buffer+idx+bytes));
        } else if (hex_buffer) {
            value = std::string((std::string_view(hex_buffer+(2*idx)+(2*bytes),len > 120 || len == 0 ? 120 : len)));
        }
        printf("  %s\n", value.c_str());
#endif
    }

    std::string HexTag() const { return DecToHex(tag, 4); }
    std::string HexGroup() const { return DecToHex(group, 2); }
    std::string HexElement() const { return DecToHex(element, 2); }
    uint64_t GetNextIndex() const { return idx + size; }

protected:
    bool require_length = false;
    bool has_reserved = false;

    uint32_t CalcLength();
    uint64_t CalcSize();
};