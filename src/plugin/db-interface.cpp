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
void DBInterface::Connect(std::string database, std::string host, uint16_t port, std::string username, std::string password) {
    try {
        char buffer[1024];
        DEBUG_LOG(DEBUG_2, "Preparing connection string");
        sprintf(buffer, "postgresql://%s:%s@%s:%hu/%s", username.c_str(), password.c_str(), host.c_str(), port, database.c_str());
        DEBUG_LOG(DEBUG_2, buffer);
        con = new pqxx::connection(buffer); // not using a pointer causes the function to throw an exception when invoked.. doesn't make any sense.. but let's just leave it as is

        DBInterface::initialize();
    } catch (const std::exception &e){
        DEBUG_LOG(PLUGIN_ERRORS, "Failed to connect to database.")
        std::cerr << e.what() << std::endl;
    }
}

void DBInterface::initialize() {
    if(con->is_open()){
        try {
            pqxx::work w(*con);
            // Create crosswalk table
            w.exec(""
                "DO $$"
                "BEGIN"
                "	BEGIN"
                "		ALTER TABLE resources ADD CONSTRAINT unique_publicid UNIQUE(publicid);"
                "	EXCEPTION"
                "		WHEN duplicate_object THEN RAISE NOTICE 'Table constraint unique_publicid already exists';"
                "	END;"
                "END $$;"

                "CREATE TABLE IF NOT EXISTS crosswalk ("
                "	internalid BIGINT NOT NULL UNIQUE,"
                "	publicid CHARACTER VARYING(64) NOT NULL UNIQUE,"
                "	patient_id TEXT,"
                "	full_name TEXT,"
                "	first_name TEXT,"
                "	middle_name TEXT,"
                "	last_name TEXT,"
                "	dob TEXT,"
                "	PRIMARY KEY (internalid),"
                "	FOREIGN KEY (internalid, publicid) REFERENCES resources(internalid, publicid) ON DELETE CASCADE"
                ");"
            );
            w.exec("CREATE INDEX IF NOT EXISTS i_patient_id ON crosswalk (patient_id);");
            w.exec("CREATE INDEX IF NOT EXISTS i_lower_full_name ON crosswalk (lower(full_name));");
            w.exec("CREATE INDEX IF NOT EXISTS i_lower_first_name ON crosswalk (lower(first_name));");
            w.exec("CREATE INDEX IF NOT EXISTS i_lower_middle_name ON crosswalk (lower(middle_name));");
            w.exec("CREATE INDEX IF NOT EXISTS i_lower_last_name ON crosswalk (lower(last_name));");
            w.exec("CREATE INDEX IF NOT EXISTS i_dob ON crosswalk (dob);");

            // Create procedures for triggers
            w.exec(""
                "CREATE OR REPLACE FUNCTION add_id_to_crosswalk() RETURNS trigger AS $add_id_to_crosswalk$"
                "	BEGIN"
                "		INSERT INTO crosswalk (internalid, publicid) VALUES (NEW.internalid, NEW.publicid);"
                "		RETURN NULL;"
                "	END;"
                "$add_id_to_crosswalk$ LANGUAGE plpgsql;"
            );
            w.exec(""
                "CREATE OR REPLACE FUNCTION add_info_to_crosswalk() RETURNS trigger AS $add_info_to_crosswalk$"
                "	DECLARE"
                "		t_first_name TEXT DEFAULT NULL;"
                "        t_middle_name TEXT DEFAULT NULL;"
                "        t_last_name TEXT DEFAULT NULL;"
                "    BEGIN"
                "		IF NEW.taggroup = 16 THEN"
                "			IF NEW.tagelement = 16 THEN"
                "				UPDATE crosswalk SET full_name = NEW.value WHERE internalid = NEW.id;"
                "				SELECT"
                "					split_part(NEW.value,' ',1),"
                "					CASE"
                "						WHEN split_part(NEW.value,' ',3) = '' THEN NULL ELSE split_part(NEW.value,' ',2) END,"
                "					CASE "
                "						WHEN split_part(NEW.value,' ',2) = '' THEN NULL "
                "						WHEN split_part(NEW.value,' ',3) = '' THEN split_part(NEW.value,' ',2) "
                "						ELSE split_part(NEW.value,' ',3) "
                "					END"
                "				INTO"
                "					t_first_name, t_middle_name, t_last_name;"
                "				UPDATE crosswalk SET first_name = t_first_name, middle_name = t_middle_name, last_name = t_last_name WHERE internalid = NEW.id;"
                "			ELSIF NEW.tagelement = 32 THEN"
                "				UPDATE crosswalk SET patient_id = NEW.value WHERE internalid = NEW.id;"
                "			ELSIF NEW.tagelement = 48 THEN"
                "				UPDATE crosswalk SET dob = NEW.value WHERE internalid = NEW.id;"
                "			END IF;"
                "		END IF;"
                "		RETURN NULL;"
                "    END;"
                "$add_info_to_crosswalk$ LANGUAGE plpgsql;"
            );

            // Create triggers
            w.exec(""
                "CREATE OR REPLACE TRIGGER new_data AFTER INSERT"
                "    ON resources"
                "	FOR EACH ROW"
                "    WHEN (NEW.resourcetype = 0)"
                "    EXECUTE PROCEDURE add_id_to_crosswalk();"
            );
            w.exec(""
                "CREATE OR REPLACE TRIGGER new_data AFTER INSERT"
                "    ON maindicomtags"
                "	FOR EACH ROW"
                "    EXECUTE PROCEDURE add_info_to_crosswalk();"
            );

            w.commit();
        }   catch (const std::exception &e) {
            DEBUG_LOG(PLUGIN_ERRORS, "Something went wrong during DB initialization");
            DEBUG_LOG(PLUGIN_ERRORS, e.what());
        }

        // Prepared Statements
        con->prepare(
                "UpdateChecksum",
                "UPDATE attachedfiles "
                "SET uncompressedsize = $1, compressedsize = $2, uncompressedhash = $3, compressedhash = $4 "
                "WHERE uuid = $5;"
        );
    }
}

void DBInterface::disconnect() {
    if(con->is_open()){
        con->close();
        delete con;
        con = nullptr;
    }
}

bool DBInterface::IsOpen() {
    return con && con->is_open();
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
    if(con && con->is_open()) {
        pqxx::work w(*con);
        w.exec_prepared("UpdateChecksum", size, size, hash, hash, uuid);
        w.commit();
    }
}

void DBInterface::CreateTables() {
    if(con && con->is_open()) {
        pqxx::work w(*con);
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
