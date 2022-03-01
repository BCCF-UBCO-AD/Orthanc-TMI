#include <dicom-checksum.h>
#include <db-interface.h>
#include <string>

std::unordered_map<const void *, std::tuple<const char *, char *>> DicomChecksum::checksum_map;

void DicomChecksum::CalculateChecksum(const char *uuid, DicomFile file) {
    char * md5 = OrthancPluginComputeMd5(globals::context, file.GetData(), file.GetSize());
    DicomChecksum::checksum_map.emplace(file.GetData(), std::make_tuple(uuid, md5));
}

void DicomChecksum::UpdateChecksum(const char *instanceId) {

}