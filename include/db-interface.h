#pragma once
#include <pqxx/pqxx>
#include <dicom-file.h>

// todo: name better?
class DBInterface { // not an 'interface'
private:
    static pqxx::connection con;
public:
    static void connect(std::string password);
    static void disconnect();
    static bool is_open() { return con.is_open(); }
    static void HandlePHI(const DicomFile &dicom);
};
