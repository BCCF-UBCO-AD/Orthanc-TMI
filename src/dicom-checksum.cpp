#include <core.h>
#include <db-interface.h>
#include <string>
#include <dicom-checksum.h>

std::unordered_map<const void *, std::tuple<std::string, std::string>> DicomChecksum::checksum_map;

void DicomChecksum::update_checksum(const char *instanceId) {
    // First we need to retrieve the internalid of the instance using instanceid from 'resource'
    // Then, using the internalid, retrieve file uuid from 'attachedfiles'
    std::string uuid = DBInterface::get_uuid_from_instanceid(instanceId);
    char msg[1024] = {0};
    sprintf(msg, "instanceId: %s, UUID: %s", instanceId, uuid.c_str());
    DEBUG_LOG(1, msg);

    // Retrieve md5 from map
    /*
    std::tuple<std::string, int> checksum = DicomChecksum::checksum_map.at(uuid);
    DicomChecksum::checksum_map.erase(uuid);
    sprintf(msg, "New MD5: %s, size: %d", std::get<0>(checksum).c_str(), std::get<1>(checksum));
    DEBUG_LOG(1, msg);
    */
    // Update checksum in 'attachedfiles'
    //DBInterface::update_checksum(uuid, std::get<1>(checksum), std::get<0>(checksum).c_str());
}

void DicomChecksum::calc_checksum(const char *uuid, char *content, int size) {
    std::string md5 = OrthancPluginComputeMd5(globals::context, content, size);
    //DicomChecksum::checksum_map.emplace(std::string(uuid), std::make_tuple(md5, size));
}
