#pragma once
#include <pqxx/pqxx>

// todo: name better?
class DBInterface { // not an 'interface'
private:
    DBInterface() = default;
public:
    //static void UpdateChecksum(std::string uuid, int64_t size, char *hash);
    static void UpdateChecksum(std::string uuid, std::string hash, int64_t size);
    static void InsertCrosswalk(std::string instance_uuid, std::string patient_id, std::string full_name, std::string dob);
    static bool Initialize();
};
