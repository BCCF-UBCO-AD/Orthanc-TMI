#pragma once
#define UNIT_TEST
extern fs::path GetProjRoot();
extern void TestWithDicomFiles(std::function<void(const fs::path&)> test);
namespace uuid {
    extern std::string generate_uuid_v4();
}