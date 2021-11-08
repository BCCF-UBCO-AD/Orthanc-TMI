#pragma once

#include <string>
#include "core.h"
extern std::string DecToHex(uint16_t value);
extern uint32_t HexToDec(std::string hex);

class DicomElement {
public:
    const char *const buffer;
    const uint64_t idx;
    const uint32_t &tag = *(uint32_t *)(buffer + idx);
    const uint16_t &group = *(uint16_t *)(buffer + idx);
    const uint16_t &element = *(uint16_t *)(buffer + idx + 2);;
    const std::string VR = std::string(std::string_view(buffer + idx + 4, 2));
    const uint32_t length;
    const uint64_t size;
    const std::string byte1;

    DicomElement(const char* data, uint64_t index)
            : buffer(data),
              idx(index),
              length(CalcLength(data, index, VR)),
              size(CalcSize(data, index, length)){}
    std::string HexTag() const { return DecToHex(group).append("," + DecToHex(element));}
    std::string HexGroup() const { return DecToHex(group); }
    std::string HexElement() const { return DecToHex(element); }
    uint64_t GetNextIndex() const { return idx + size; }
protected:
    bool require_length = false;
    bool has_reserved = false;
    size_t bytes = 0;
    uint32_t CalcLength(const char *data, uint64_t idx, std::string VR) {
        require_length = VR == "UT";
        has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
        uint32_t value;
        if (has_reserved) {
            bytes = 12;
            value = *(uint32_t *) (data + idx + 8); // 4 bytes
        } else {
            bytes = 8;
            value = *(uint16_t *) (data + idx + 6); // 4 bytes
        }
        return value == -1 ? 0 : value;
    }

    uint64_t CalcSize(const char* data, uint64_t idx, uint32_t length) {
        //runs after CalcLength
        return length + bytes;
    }
};