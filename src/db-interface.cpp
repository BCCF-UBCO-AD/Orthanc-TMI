#include "iostream"
#include "db-interface.h"
#include "pqxx/except.hxx"

void DBInterface::connect(const char* password) {

    try{
        char buffer[256];
        sprintf(buffer,"postgresql://postgres:%s@localhost:5432", password);
        con = pqxx::connection(buffer);
    } catch (pqxx::broken_connection const &e) {
        std::cerr << "Connection Error: " << e.what() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void DBInterface::disconnect() {
    if(con.is_open()){
        con.close();
    }
}
