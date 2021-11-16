#include <dicom-element.h>
#include <dicom-tag.h>

uint32_t DicomElement::CalcLength() {
    // length width, and location must be deduced
    // Dicom Element Structure: https://www.leadtools.com/help/sdk/v21/dicom/api/overview-data-element-structure.html#!
    require_length = VR == "UT";
    has_reserved = require_length || VR == "OB" || VR == "SQ" || VR == "OW" || VR == "UN";
    uint32_t value;

    if (has_reserved) {
        // structure includes reserved bytes after the VR
        bytes = 12;
        value = *(uint32_t *) (buffer + idx + 8); // 4 bytes
    } else if(tag == dicomTag::Item){
        // structure has an implicit VR
        bytes = 8;
        value = *(uint32_t *) (buffer + idx + 4); // 4 bytes
    } else {
        // structure with an explicit VR but no reserved bytes
        bytes = 8;
        value = *(uint16_t *) (buffer + idx + 6); // 2 bytes
    }
    return value;
}

uint64_t DicomElement::CalcSize() {
    //runs after CalcLength
    if (length == -1) {
        // Dicom Element is only as big as the header
        return bytes;
    }
    // Dicom Element has an explicit value length
    return length + bytes;
}


