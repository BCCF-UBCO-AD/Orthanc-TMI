#pragma once
#include <orthanc/OrthancCPlugin.h>
#include <unordered_set>

#ifndef IMPLEMENTS_GLOBALS
namespace globals {
    extern OrthancPluginContext* context;
    extern std::unordered_set<uint32_t> filter_list;
}
#endif