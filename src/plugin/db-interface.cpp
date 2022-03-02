#include <core.h>
#include <db-interface.h>
#include <pqxx/except.hxx>
#include <iostream>

//https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-CONNSTRING
//postgresql://[userspec@][hostspec][/dbname][?paramspec]
//    where userspec is:
//     user[:password]
//    and hostspec is:
//     [host][:port][,...]
//    and paramspec is:
//     name=value[&...]
// ex:
//  postgresql://user:secret@localhost
//  postgresql://other@localhost/otherdb?connect_timeout=10&application_name=myapp
//  postgresql://host1:123,host2:456/somedb?target_session_attrs=any&application_name=myapp
void DBInterface::Connect(std::string database, std::string host, std::string port, std::string username, std::string password) {
    char buffer[256];
    sprintf(buffer, "postgresql://%s:%s@%s:%s/%s", username.c_str(), password.c_str(), host.c_str(), port.c_str(), database.c_str());
    try {
        con = pqxx::connection(buffer);
//            con.prepare(
//                    "get_uuid_from_instanceid",
//                    "SELECT a.uuid "
//                    "FROM attachedfiles a, resources r "
//                    "WHERE r.internalid = a.id AND r.publicid = $1;"
//                    );
        con.prepare(
                "UpdateChecksum",
                "UPDATE attachedfiles "
                "SET uncompressedsize = $1, compressedsize = $2, uncompressedhash = $3, compressedhash = $4 "
                "WHERE uuid = $5;"
                );
    } catch (const std::exception &e){
        DEBUG_LOG(PLUGIN_ERRORS, "Failed to connect to database.")
        std::cerr << e.what() << std::endl;
    }
}

void DBInterface::disconnect() {
    if(con.is_open()){
        con.close();
    }
}

bool DBInterface::IsOpen() {
    return con.is_open();
}

//std::string DBInterface::get_uuid_from_instanceid(const char* instanceid) {
//    pqxx::work w( *con);
//    pqxx::result r = w.exec_prepared("get_uuid_from_instanceid", instanceid);
//
//    const pqxx::row row = r[0];
//    const pqxx::field field = row[0];
//    return std::string(field.c_str());
//}


void DBInterface::UpdateChecksum(std::string uuid, int64_t size, const char* hash) {
    if(con.is_open()) {
        pqxx::work w(con);
        w.exec_prepared("UpdateChecksum", size, size, hash, hash, uuid);
        w.commit();
    }
    /*
    pqxx::result r = w.exec("SELECT * FROM attachedfiles;");
    DEBUG_LOG(1, "Current checksum in database: ");
    DEBUG_LOG(1, "| id | filetype | uuid | compressedsize | uncompressedsize | compressiontype | uncompressedhash | compressedhash | revision |");
    for (auto const &row: r)
    {
        std::string rs = "| ";
        for (auto const &field: row) {
            rs += field.c_str();
            rs += " | ";
        }
        DEBUG_LOG(1, rs.c_str());
    }*/
}


void DBInterface::CreateTables() {
    if(con.is_open()) {
        pqxx::work w(con);
        w.exec0("CREATE SEQUENCE IF NOT EXISTS public.id_sequence\n"
                "INCREMENT 1\n"
                "START 1\n"
                "MINVALUE 1;\n"
                "CREATE TABLE IF NOT EXISTS public.dicom_instances (\n"
                "    uuid VARCHAR PRIMARY KEY,\n"
                "    timestamp int NOT NULL\n"
                ");\n"
                "\n"
                "CREATE TABLE IF NOT EXISTS public.dicom_identifiers (\n"
                "    id INT DEFAULT nextval('id_sequence'::regclass) NOT NULL,\n"
                "    uuid VARCHAR NOT NULL,\n"
                "    tag_group VARCHAR NOT NULL,\n"
                "    tag_element VARCHAR NOT NULL,\n"
                "    value VARCHAR,\n"
                "    PRIMARY KEY (id),\n"
                "    FOREIGN KEY (uuid) REFERENCES public.dicom_instances(uuid)\n"
                ");\n"
                "\n"
                "CREATE TABLE IF NOT EXISTS public.dicom_duplicates (\n"
                "    uuid VARCHAR PRIMARY KEY,\n"
                "    parent_uuid VARCHAR,\n"
                "    FOREIGN KEY (uuid) REFERENCES public.dicom_instances(uuid),\n"
                "    FOREIGN KEY (parent_uuid) REFERENCES public.dicom_instances(uuid)\n"
                ");\n"
               );
        w.commit();
    }
}
