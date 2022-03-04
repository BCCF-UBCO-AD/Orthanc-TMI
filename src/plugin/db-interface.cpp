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
            w.exec(
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
                    "	FOREIGN KEY (internalid) REFERENCES resources(internalid),"
                    "	FOREIGN KEY (publicid) REFERENCES resources(publicid)"
                    ");"
            );
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (patient_id);");
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (lower(full_name));");
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (lower(first_name));");
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (lower(middle_name));");
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (lower(last_name));");
            w.exec("CREATE INDEX IF NOT EXISTS ON crosswalk (dob);");

            // Create functions
            w.exec(
                    "CREATE OR REPLACE FUNCTION add_patient_to_crosswalk(uuid text) RETURNS BOOLEAN AS $$"
                    "	DECLARE"
                    "		intern_id BIGINT DEFAULT NULL;"
                    "		resource_type INT DEFAULT NULL;"
                    "		patient_id TEXT DEFAULT NULL;"
                    "		full_name TEXT DEFAULT NULL;"
                    "        first_name TEXT DEFAULT NULL;"
                    "        middle_name TEXT DEFAULT NULL;"
                    "        last_name TEXT DEFAULT NULL;"
                    "        dob TEXT DEFAULT NULL;"
                    "	BEGIN"
                    "		SELECT r.internalid, r.resourcetype INTO intern_id, resource_type FROM resources r WHERE r.publicid = uuid LIMIT 1;"
                    "		IF intern_id IS NOT NULL AND resource_type = 0 THEN"
                    "			/* Patient ID */"
                    "			SELECT value INTO patient_id"
                    "				FROM maindicomtags"
                    "				WHERE id = intern_id"
                    "				  AND taggroup = 16"
                    "				  AND tagelement = 32; "
                    "			/* Full Name */"
                    "			SELECT value INTO full_name"
                    "				FROM maindicomtags"
                    "				WHERE id = intern_id"
                    "				  AND taggroup = 16"
                    "				  AND tagelement = 16;"
                    "			/* Patient DOB */"
                    "			SELECT value INTO dob"
                    "				FROM maindicomtags"
                    "				WHERE id = intern_id"
                    "				  AND taggroup = 16"
                    "				  AND tagelement = 48;"
                    "			/* Split patient name into First/Middle/Last name */"
                    "			IF full_name IS NOT NULL THEN"
                    "				SELECT"
                    "				   split_part(full_name,' ',1),"
                    "				   CASE WHEN split_part(full_name,' ',3) = '' THEN NULL ELSE split_part(full_name,' ',2) END,"
                    "				   CASE "
                    "				   	WHEN split_part(full_name,' ',2) = '' THEN NULL "
                    "				   	WHEN split_part(full_name,' ',3) = '' THEN split_part(full_name,' ',2) "
                    "					ELSE split_part(full_name,' ',3) "
                    "					END"
                    "				INTO"
                    "					first_name, middle_name, last_name;"
                    "			END IF;"
                    "			INSERT INTO crosswalk VALUES (intern_id, uuid, patient_id, full_name, first_name, middle_name, last_name, dob) ON CONFLICT DO NOTHING;"
                    "			/* RAISE EXCEPTION  'full_name: %, dob: %, first_name: %, middle_name: %, last_name: %', full_name, dob, first_name, middle_name, last_name; */"
                    "			RETURN TRUE;"
                    "		END IF;"
                    "		RETURN FALSE;"
                    "	END;"
                    "$$ LANGUAGE plpgsql;"
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
        con->prepare(
                "UpdateCrosswalk",
                "SELECT add_patient_to_crosswalk($1);"
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

void DBInterface::UpdateCrosswalk(const char *resourceId) {
    if(con && con->is_open()) {
        pqxx::work w(*con);
        w.exec_prepared("UpdateCrosswalk", resourceId);
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
