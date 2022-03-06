#pragma once
#include <pqxx/pqxx>

// todo: name better?
class DBInterface { // not an 'interface'
private:
    DBInterface() = default;
public:
    //static void UpdateChecksum(std::string uuid, int64_t size, char *hash);
    static void UpdateChecksum(std::string uuid, std::string hash, int64_t size);
    static bool Initialize();
};
