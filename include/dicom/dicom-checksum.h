#pragma once
#include <core.h>
#include <dicom-file.h>
#include <mutex>

using Uuid = std::string;
using Md5 = std::string;

class DicomChecksum {
private:
    static std::mutex map_lock;
    static std::unordered_map<const void*, std::tuple<Uuid, Md5, size_t>> checksum_map;
public:
    static void SaveChecksum(const void* instance_data, std::string uuid, std::string md5, size_t size);
    static bool UpdateDatabase(const void* instance_data);
};