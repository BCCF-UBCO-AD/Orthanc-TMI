#include "db-interface.h"
#include "pqxx/except.hxx"

pqxx::connection DBInterface::con;

void DBInterface::connect(std::string password) {
    if(!con.is_open()) {
        char buffer[256];
        sprintf(buffer, "postgresql://postgres:%s@localhost:5432", password.c_str());
        con = pqxx::connection(buffer);
    }
}

void DBInterface::disconnect() {
    if(con.is_open()){
        con.close();
    }
}

void DBInterface::HandlePHI(const DicomFile &dicom) {

}