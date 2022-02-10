#pragma once
#include <pqxx/pqxx>
#include <dicom-file.h>

// todo: name better?
class DBInterface { // not an 'interface'
public:
    static void connect(std::string host, std::string password);
    static void disconnect();
    static bool is_open();
    static void HandlePHI(const DicomFile &dicom);
    
    //static void UpdateChecksum(std::string uuid, int64_t size, char *hash);
    static void create_tables();

    static std::string get_uuid_from_instanceid(const char *instanceid);
    static void update_checksum(std::string uuid, int64_t size, const char *hash);
};
