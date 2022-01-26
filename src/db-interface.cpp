#include "db-interface.h"
#include "pqxx/except.hxx"
#include "iostream"

pqxx::connection* con = nullptr;

void DBInterface::connect(std::string host, std::string password) {
    if(!con) {
        char buffer[256];
        //todo: we should build the connection string based on the json config
        sprintf(buffer, "postgresql://postgres:%s@%s:5432/orthanc", password.c_str(), host.c_str());
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
