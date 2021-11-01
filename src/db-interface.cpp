#include "iostream"
#include "db-interface.h"
#include "pqxx/except.hxx"

void DBInterface::connect(const char* password) {

    try{
        char buffer[256];
        sprintf(buffer,"postgresql://postgres:%s@localhost:5432", password);
        con = pqxx::connection(buffer);
    } catch (pqxx::broken_connection const &e) {
        // connection checks itself for is_open(), this is thrown if the return was false
        std::cerr << "Connection Error: " << e.what() << std::endl;
    } catch (std::exception const &e) {
        // other exceptions can be thrown from ctor
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void DBInterface::disconnect() {
    if(con.is_open()){
        con.close();
    }
}
