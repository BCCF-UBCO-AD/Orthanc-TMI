#include <dicom-checksum.h>
#include <db-interface.h>
#include <string>

std::unordered_map<const void *, std::tuple<std::string, std::string, size_t>> DicomChecksum::checksum_map;
std::mutex DicomChecksum::map_lock;

void DicomChecksum::SaveChecksum(const void* instance_data, std::string uuid, std::string md5, size_t size) {
    map_lock.lock();
    checksum_map.emplace(instance_data, std::make_tuple(uuid, md5, size));
    map_lock.unlock();
}

bool DicomChecksum::UpdateDatabase(const void* instance_data) {
    char msg[1024];
    map_lock.lock();
    auto iter = checksum_map.find(instance_data);
    if (iter != checksum_map.end()) {
        auto &[uuid, md5, size] = iter->second;
        sprintf(msg, "UUID: %s, MD5: %s", uuid.c_str(), md5.c_str());
        DBInterface::UpdateChecksum(uuid, size, md5.c_str());
        DEBUG_LOG(1, msg);
        checksum_map.erase(iter);
        map_lock.unlock();
        return true;
    }
    // Checksum not found in checksum_map
    map_lock.unlock();
    sprintf(msg, "DicomChecksum::UpdateDatabase: Failure! No checksum was found for %zu.", instance_data);
    DEBUG_LOG(PLUGIN_ERRORS, msg);
    return false;
}