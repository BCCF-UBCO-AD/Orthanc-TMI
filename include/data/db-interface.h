#pragma once
#include <pqxx/pqxx>
#include <dicom-file.h>

// todo: name better?
class DBInterface { // not an 'interface'
public:
    static void connect(std::string database, std::string host, std::string port, std::string username, std::string password);
    static void disconnect();
    static bool is_open();
    
    //static void UpdateChecksum(std::string uuid, int64_t size, char *hash);
    static void CreateTables();
    static void UpdateChecksum(std::string uuid, int64_t size, const char *hash);
};
