#pragma once
#include <pqxx/pqxx>

// todo: name better?
class DBInterface { // not an 'interface'
private:
    pqxx::connection* con = nullptr;
    DBInterface() = default;
    void initialize();
    void disconnect();
public:
    ~DBInterface() {
        disconnect();
    }
    static DBInterface& GetInstance(){
        static DBInterface instance;
        return instance;
    }
    void Connect(std::string database, std::string host, uint16_t port, std::string username, std::string password);
    bool IsOpen();
    
    //static void UpdateChecksum(std::string uuid, int64_t size, char *hash);
    void UpdateChecksum(std::string uuid, std::string hash, int64_t size);
};
