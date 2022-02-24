#include <dicom-element-view.h>
#include <dicom-tag.h>

uint64_t DicomElementView::CalcValueOffset() {
    // length width, and location must be deduced
    // Dicom Element Structure: https://www.leadtools.com/help/sdk/v21/dicom/api/overview-data-element-structure.html#!
    require_length = VR == "UT";
    has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
    if (has_reserved) {
        // structure includes reserved bytes after the VR
        return 12;
    } else if(tag == DicomTag::Item){
        // structure has an implicit VR
        return 8;
    }
    return 8;
}

uint32_t DicomElementView::CalcValueLength() {
    // length width, and location must be deduced
    // Dicom Element Structure: https://www.leadtools.com/help/sdk/v21/dicom/api/overview-data-element-structure.html#!
    require_length = VR == "UT";
    has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
    uint32_t value;

    if (has_reserved) {
        // structure includes reserved bytes after the VR
        std::memcpy(&value, buffer + idx + 8, sizeof(uint32_t)); // 4 bytes
    } else if(tag == DicomTag::Item){
        // structure has an implicit VR
        std::memcpy(&value, buffer + idx + 4, sizeof(uint32_t)); // 4 bytes
    } else {
        // structure with an explicit VR but no reserved bytes
        uint16_t t;
        std::memcpy(&t, buffer + idx + 6, sizeof(uint16_t)); // 2 bytes
        value = t;
    }
    return value;
}

uint64_t DicomElementView::CalcElementSize() {
    //runs after CalcLength
    if (value_length == -1) {
        // Dicom Element is only as big as the header
        return value_offset;
    }
    // Dicom Element has an explicit value length
    return value_offset + value_length;
}

uint32_t DicomElementView::ReadTag() {
    uint32_t result;
    std::memcpy(&result, buffer + idx, sizeof(uint32_t));
    return result;
}
