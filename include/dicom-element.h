#pragma once
#include <string>
#include "core.h"
extern std::string DecToHex(uint64_t value);
extern uint32_t HexToDec(std::string hex);

class DicomElement {
public:
    const char *const buffer;
    const uint64_t idx;
    const uint32_t &tag = *(uint32_t *)(buffer + idx);
    const uint16_t &group = *(uint16_t *)(buffer + idx);
    const uint16_t &element = *(uint16_t *)(buffer + idx + 2);;
    const std::string VR = std::string(std::string_view(buffer + idx + 4, 2));
    const uint32_t length = CalcLength();
    const uint64_t size = CalcSize();

    DicomElement(const char* buffer, uint64_t index, const char* hex_buffer = nullptr) : buffer(buffer), idx(index){
#ifdef DEBUG_
        printf("[%s](%s,%s)->(%s)\n",
               VR.c_str(),
               HexGroup().c_str(),
               HexElement().c_str(),
               HexTag().c_str());
        printf(" idx: %zu, size: %zu, bytes: %zu, length: %zu\n",idx,size,bytes,length);
        std::string value;
        uint32_t len = length == -1 ? 1 : length;
        if(VR == "UL") {
            value = std::to_string(*(uint32_t*)(buffer+idx+bytes));
        } else if (hex_buffer) {
            value = std::string((std::string_view(hex_buffer+(2*idx)+(2*bytes),12)));
        }
        printf("  %s\n", value.c_str());
#endif
    }
    std::string HexTag() const { return DecToHex(tag);}
    std::string HexGroup() const { return DecToHex(group); }
    std::string HexElement() const { return DecToHex(element); }
    uint64_t GetNextIndex() const { return idx + size; }
protected:
    bool require_length = false;
    bool has_reserved = false;
    size_t bytes = 0;
    uint32_t CalcLength() {
        require_length = VR == "UT";
        has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
        uint32_t value;
        if (has_reserved) {
            bytes = 12;
            value = *(uint32_t *) (buffer + idx + 8); // 4 bytes
        } else {
            bytes = 8;
            value = *(uint16_t *) (buffer + idx + 6); // 4 bytes
        }
        return value;
    }

    uint64_t CalcSize() {
        //runs after CalcLength
        if(length == -1){
#ifdef DEBUG_
            printf("calc size: invalid length\n");
#endif
            //todo: search for delimiter
            DicomElement element(buffer,idx+bytes);
#ifdef DEBUG_
            printf("calc size: %zu\n", bytes + element.size);
            printf("(%s)=>%d [%s] - value length: %d - (header: %zu Bytes; size: %zu)(index=%zu->%zu)\n", element.HexTag().c_str(), element.tag, element.VR.c_str(), element.length, bytes, element.size, element.idx, element.GetNextIndex());
#endif
            return element.size + bytes;
        }
        return length + bytes;
    }
};