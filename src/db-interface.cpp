#include "db-interface.h"
#include "pqxx/except.hxx"
#include "iostream"

pqxx::connection* con = nullptr;

void DBInterface::connect(std::string password) {
    if(!con) {
        char buffer[256];
        //todo: we should build the connection string based on the json config
        sprintf(buffer, "postgresql://postgres:%s@postgres:5432", password.c_str());
        try {
            static pqxx::connection c(buffer);
            con = &c;
        } catch (const std::exception &e){
            std::cerr << e.what() << std::endl;
        }
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

void DBInterface::create_tables() {

}
