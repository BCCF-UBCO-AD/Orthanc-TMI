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
    static void HandlePHI(const DicomFile &dicom);
};
