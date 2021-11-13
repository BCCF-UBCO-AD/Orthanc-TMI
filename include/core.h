#pragma once
#include <orthanc/OrthancCPlugin.h>
#include <unordered_set>
#include <filesystem>
#include <string>

using TagFilter = std::unordered_set<uint32_t>;
namespace fs = std::filesystem;

#ifndef IMPLEMENTS_GLOBALS
namespace globals {
    extern OrthancPluginContext* context;
    extern TagFilter filter_list;
    extern std::string storage_location;
    extern fs::perms dir_permissions;
    extern fs::perms file_permissions;
}
#endif