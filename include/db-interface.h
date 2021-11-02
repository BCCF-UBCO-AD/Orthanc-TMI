#pragma once
#include <pqxx/pqxx>

// todo: name better?
class DBInterface { // not an 'interface'
private:
    static pqxx::connection con;
public:
    static void connect(std::string password);
    static void disconnect();
};
