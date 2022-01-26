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
    extern fs::perms dir_permissions;
    extern fs::perms file_permissions;
}
#endif
#ifndef NDEBUG
    #ifndef UNIT_TEST
     #define DEBUG_LOG(msg) if(globals::context) OrthancPluginLogWarning(globals::context, msg);
    #else
     #include <iostream>
     #define DEBUG_LOG(msg) std::cout << msg << std::endl;
    #endif
#else
 #define DEBUG_LOG(msg)
#endif
