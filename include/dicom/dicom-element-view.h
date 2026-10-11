#pragma once
#include <core.h>
#include <dicom-tag.h>

/* class: DicomElementView
 *  Used to view a dicom element and its data. Could be used to implement an iterator.
 *
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
class DicomElementView {
    // below DicomElement locates and aliases data within the dicom's data buffer, then calculates appropriate length/size values
public:
    const char* const buffer;
    const char* const hex_buffer;
    const uint64_t idx;
    // these are apparently UB, according to the experts on TPH (discord) (ie. `*(uint32_t*) (buffer + idx);`)
    const uint32_t tag = ReadTag();
    const uint16_t group = tag&GROUP_MASK;
    const uint16_t element = tag-group;
    // end of UB, hopefully
    const std::string VR = std::string(std::string_view(buffer + idx + 4, 2));
    const uint64_t value_offset = CalcValueOffset();
    const uint32_t value_length = CalcValueLength();
    const uint64_t size = CalcElementSize();
    //const std::string value = std::string(std::string_view(buffer + idx + value_offset, value_length));

    DicomElementView(const void* buffer, uint64_t index, const char* hex_buffer = nullptr)
            : buffer((const char*) buffer),
              hex_buffer(hex_buffer),
              idx(index) {}

    std::string HexTag() const { return DecToHex(tag, 4); }
    std::string HexGroup() const { return DecToHex(group, 2); }
    std::string HexElement() const { return DecToHex(element, 2); }
    uint64_t GetNextIndex() const { return idx + size; }
    uint64_t GetValueIndex() const { return idx + value_offset; }
    const char* GetValueHead() const { return buffer + idx + value_offset; }
protected:
    bool require_length = false;
    bool has_reserved = false;

    uint64_t CalcValueOffset();
    uint32_t CalcValueLength();
    uint64_t CalcElementSize();
    uint32_t ReadTag();
};