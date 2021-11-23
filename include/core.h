#pragma once
#include <orthanc/OrthancCPlugin.h>
#include <unordered_set>
#include <filesystem>
#include <memory>
#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace nlm = nlohmann;
namespace fs = std::filesystem;

// plugin.cpp externs
#ifndef IMPLEMENTS_GLOBALS
namespace globals {
    extern OrthancPluginContext* context;
    extern std::string storage_location;
    extern nlm::json config;
    extern fs::perms dir_permissions;
    extern fs::perms file_permissions;
}
#endif