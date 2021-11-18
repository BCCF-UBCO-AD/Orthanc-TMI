#include "db-interface.h"
#include "pqxx/except.hxx"

pqxx::connection* con = nullptr;

void DBInterface::connect(std::string password) {
    if(!con) {
        char buffer[256];
        sprintf(buffer, "postgresql://postgres:%s@localhost:5432", password.c_str());
        static pqxx::connection c(buffer);
        con = &c;
    }
}

void DBInterface::disconnect() {
    if(con && con->is_open()){
        con->close();
    }
}

bool DBInterface::is_open() {
    return con && con->is_open();
}

void DBInterface::HandlePHI(const DicomFile &dicom) {

}