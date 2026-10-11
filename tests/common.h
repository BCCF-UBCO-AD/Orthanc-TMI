#pragma once
//commented out for CI (uncomment for local testing)
//#define UNIT_TEST
#include <core.h>
#include <functional>

extern fs::path GetProjRoot();
extern void TestWithDicomFiles(std::function<void(const fs::path&)> test);
namespace uuid {
    extern std::string generate_uuid_v4();
}
