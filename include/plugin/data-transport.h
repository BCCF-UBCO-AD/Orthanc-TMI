#pragma once
#include <core.h>

#include <mutex>
#include <unordered_map>

using Uuid = std::string;
using Md5 = std::string;

class DataTransport {
private:
    static std::mutex checksum_lock;
    static std::mutex uuid_lock;
    static std::unordered_map<const void*, std::tuple<Md5, size_t>> checksum_map;
    static std::unordered_map<const void*, Uuid> uuid_map;
public:
    static void Emplace(const void* instance_data, std::string md5, size_t size);
    static void Emplace(const void* instance_data, std::string uuid);
    static bool UpdateDatabase(const void* instance_data);
};