#include "db-interface.h"

void DBInterface::connect(const char* password) {
    if(!con.is_open()){
        char buffer[256];
        sprintf(buffer,"postgresql://postgres:%s@localhost:5432", password);
        con = pqxx::connection(buffer);
        if(!con.is_open()){
            // todo: error
        }
    } else {
        // todo: error
    }
}

void DBInterface::disconnect() {
    if(con.is_open()){
        con.close();
    }
}
