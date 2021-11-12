#pragma once
#include <orthanc/OrthancCPlugin.h>
#include <unordered_set>
#include <string>

using TagFilter = std::unordered_set<uint32_t>;

#ifndef IMPLEMENTS_GLOBALS
namespace globals {
    extern OrthancPluginContext* context;
    extern TagFilter filter_list;
    extern std::string storage_location;
}
#endif