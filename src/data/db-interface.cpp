#include "db-interface.h"
#include "pqxx/except.hxx"
#include "iostream"

pqxx::connection* con = nullptr;

void DBInterface::connect(std::string host, std::string password) {
    if(!con) {
        char buffer[256];
        //todo: replace "postgres:"->"%s" and pass user to connect function
        sprintf(buffer, "postgresql://postgres:%s@%s:5432/orthanc", password.c_str(), host.c_str());
        try {
            static pqxx::connection c(buffer);
            con = &c;
            con->prepare(
                    "get_uuid_from_instanceid",
                    "SELECT a.uuid "
                    "FROM attachedfiles a, resources r "
                    "WHERE r.internalid = a.id AND r.publicid = $1;"
                    );
            con->prepare(
                    "update_checksum",
                    "UPDATE attachedfiles "
                    "SET uncompressedsize = $1, compressedsize = $2, uncompressedhash = $3, compressedhash = $4 "
                    "WHERE uuid = $5;"
                    );
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

std::string DBInterface::get_uuid_from_instanceid(const char* instanceid) {
    pqxx::work w( *con);
    pqxx::result r = w.exec_prepared("get_uuid_from_instanceid", instanceid);

    const pqxx::row row = r[0];
    const pqxx::field field = row[0];
    return std::string(field.c_str());
}


void DBInterface::update_checksum(std::string uuid, int64_t size, const char* hash) {
    pqxx::work w( *con);
    w.exec_prepared("update_checksum", size, size, hash, hash, uuid);
    w.commit();
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


void DBInterface::create_tables() {
    pqxx::work w( *con);
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
