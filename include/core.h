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

#define PLUGIN_ERRORS (-1)
#define INFO 0
#define VERBOSE_1 1
#define VERBOSE_2 2

#ifndef NDEBUG
    #define LOGGING_LEVEL VERBOSE_1
    #ifndef UNIT_TEST
     #define DEBUG_LOG(L,msg) if(globals::context && L <= LOGGING_LEVEL) switch(L){ \
        case PLUGIN_ERRORS: OrthancPluginLogError(globals::context, msg);break;     \
        case INFO: OrthancPluginLogWarning(globals::context, msg); break;           \
        case VERBOSE_1:                                                             \
        case VERBOSE_2: OrthancPluginLogInfo(globals::context, msg); break;         \
      }
    #else
     #define LOGGING_LEVEL INFO
     #include <iostream>
     #define DEBUG_LOG(L,msg) if(L <= LOGGING_LEVEL) printf("%s\n",msg);
    #endif
#else
  #define LOGGING_LEVEL INFO
  #define DEBUG_LOG(L,msg) if(globals::context && L <= LOGGING_LEVEL) switch(L){ \
     case PLUGIN_ERRORS: OrthancPluginLogError(globals::context, msg);break;     \
     case INFO: OrthancPluginLogWarning(globals::context, msg); break;           \
   }
#endif
