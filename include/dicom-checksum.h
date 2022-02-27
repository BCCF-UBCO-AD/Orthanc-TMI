#pragma once
#include <core.h>

class DicomChecksum {
public:
    static std::unordered_map<const void *, std::tuple<std::string, std::string>> checksum_map;
    static void update_checksum(const char *instanceId);
    static void calc_checksum(const char *uuid, char *content, int size);
};