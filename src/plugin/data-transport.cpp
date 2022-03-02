#include <data-transport.h>
#include <db-interface.h>

std::unordered_map<const void *, std::tuple<std::string, size_t>> DataTransport::checksum_map;
std::unordered_map<const void *, std::string> DataTransport::uuid_map;
std::mutex DataTransport::checksum_lock;
std::mutex DataTransport::uuid_lock;

void DataTransport::Emplace(const void* instance_data, std::string md5, size_t size) {
    checksum_lock.lock();
    checksum_map.emplace(instance_data, std::make_tuple(md5, size));
    checksum_lock.unlock();
}

void DataTransport::Emplace(const void* instance_data, std::string uuid) {
    uuid_lock.lock();
    uuid_map.emplace(instance_data, uuid);
    uuid_lock.unlock();
}

bool DataTransport::UpdateDatabase(const void* instance_data) {
    char msg[1024];
    checksum_lock.lock();
    auto checksum_iter = checksum_map.find(instance_data);
    uuid_lock.lock();
    auto uuid_iter = uuid_map.find(instance_data);
    if (checksum_iter != checksum_map.end() && uuid_iter != uuid_map.end()) {
        auto &[md5, size] = checksum_iter->second;
        auto uuid = uuid_iter->second;
        sprintf(msg, "UUID: %s, MD5: %s", uuid.c_str(), md5.c_str());
        DBInterface::GetInstance().UpdateChecksum(uuid, size, md5.c_str());
        DEBUG_LOG(1, msg);
        checksum_map.erase(checksum_iter);
        checksum_lock.unlock();
        uuid_map.erase(uuid_iter);
        uuid_lock.unlock();
        return true;
    }
    // Checksum not found in checksum_map
    checksum_lock.unlock();
    sprintf(msg, "DicomChecksum::UpdateDatabase: Failure! No checksum was found for %zu.", instance_data);
    DEBUG_LOG(PLUGIN_ERRORS, msg);
    return false;
}