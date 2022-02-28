#pragma once
#include "core.h"

class DicomChecksum {
public:
    static std::unordered_map<const void *, std::tuple<const char *, char *>> checksum_map;

    static void CalculateChecksum(const char *uuid, DicomFile file);
    static void UpdateChecksum(const char *instanceId);
};