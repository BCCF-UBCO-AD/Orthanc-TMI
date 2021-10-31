#pragma once
#include "core.h"

// todo: rename? eg. TagFilter? *Parser?
class DicomFilter{
private:
    const OrthancPluginDicomInstance* original_instance;
    const void* data;
    uint64_t size;
protected:
public:
    DicomFilter(const OrthancPluginDicomInstance* readonly_instance);
    const OrthancPluginDicomInstance* GetFilteredInstance();
};