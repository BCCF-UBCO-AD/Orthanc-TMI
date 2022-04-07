#include <core.h>
#include <db-interface.h>
#include <plugin-configure.h>
#include <pqxx/except.hxx>
#include <mutex>

void DBInterface::UpdateChecksum(std::string uuid, std::string hash, int64_t size) {
    try {
        static pqxx::connection con(PluginConfigurer::GetDBConnectionInfo());
        if (con.is_open()) {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                // Prepared Statements
                con.prepare(
                        "UpdateChecksum",
                        "UPDATE attachedfiles "
                        "SET uncompressedsize = $1, compressedsize = $1, uncompressedhash = $2, compressedhash = $2 "
                        "WHERE uuid = $3;"
                           );
            });
            pqxx::work w(con);
            w.exec_prepared("UpdateChecksum", size, hash, uuid);
            w.commit();
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "UpdateChecksum(): couldn't connect to database.");
        }
    } catch (const std::exception &e){
        DEBUG_LOG(PLUGIN_ERRORS, "UpdateChecksum() encountered an exception");
        DEBUG_LOG(PLUGIN_ERRORS, e.what());
    }
}

void DBInterface::InsertCrosswalk(std::string instance_uuid, std::string patient_id, std::string full_name, std::string dob) {
    try {
        static pqxx::connection con(PluginConfigurer::GetDBConnectionInfo());
        if (con.is_open()) {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                // Prepared Statements
                con.prepare(
                        "InsertCrosswalk",
                        "SELECT insert_info_crosswalk($1, $2, $3, $4)"
                );
            });
            pqxx::work w(con);
            w.exec_prepared("InsertCrosswalk", instance_uuid, patient_id, full_name, dob);
            w.commit();
        } else {
            DEBUG_LOG(PLUGIN_ERRORS, "InsertCrosswalk(): couldn't connect to database.");
        }
    } catch (const std::exception &e){
        DEBUG_LOG(PLUGIN_ERRORS, "InsertCrosswalk() encountered an exception");
        DEBUG_LOG(PLUGIN_ERRORS, e.what());
    }
}

bool DBInterface::Initialize() {
    try {
        static std::once_flag flag;
        bool return_value = false;
        std::call_once(flag, [&]() {
            pqxx::connection con(PluginConfigurer::GetDBConnectionInfo());
            if (con.is_open()) {
                pqxx::work w(con);

                // Create crosswalk table
                w.exec(""
                       "CREATE TABLE IF NOT EXISTS crosswalk ("
                       "	id SERIAL NOT NULL,"
                       "	internalid BIGINT NOT NULL,"
                       "	publicid CHARACTER VARYING(64) NOT NULL,"
                       "	patient_id TEXT NOT NULL,"
                       "	full_name TEXT NOT NULL,"
                       "	first_name TEXT,"
                       "	middle_name TEXT,"
                       "	last_name TEXT,"
                       "	dob TEXT NOT NULL,"
                       "    instances TEXT[],"
                       "	PRIMARY KEY (id),"
                       "    UNIQUE (internalid, publicid, patient_id, full_name, dob)"
                       ");"
                      );

                // Create mismatch table
                w.exec(""
                       "CREATE TABLE IF NOT EXISTS phi_mismatch("
                       "    id SERIAL NOT NULL,"
                       "    internalid BIGINT NOT NULL UNIQUE,"
                       "    publicid CHARACTER VARYING(64) NOT NULL UNIQUE,"
                       "    parent_internalid BIGINT NOT NULL,"
                       "    parent_publicid CHARACTER VARYING(64) NOT NULL,"
                       "    PRIMARY KEY (id)"
                       ");"
                      );

                // Create indexes
                w.exec("CREATE INDEX IF NOT EXISTS i_internalid ON crosswalk (internalid);");
                w.exec("CREATE INDEX IF NOT EXISTS i_publicid ON crosswalk (publicid);");
                w.exec("CREATE INDEX IF NOT EXISTS i_patient_id ON crosswalk (patient_id);");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_full_name ON crosswalk (lower(full_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_first_name ON crosswalk (lower(first_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_middle_name ON crosswalk (lower(middle_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_last_name ON crosswalk (lower(last_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_dob ON crosswalk (dob);");

                // Create procedures for triggers
                w.exec(""
                       "CREATE OR REPLACE FUNCTION insert_info_crosswalk(v_instance_uuid TEXT, v_patient_id TEXT, v_full_name TEXT, v_dob TEXT) RETURNS void AS $insert_info_crosswalk$"
                       "    DECLARE"
                       "    t_internalid BIGINT;"
                       "        t_patient_uuid TEXT DEFAULT NULL;"
                       "        t_first_name TEXT DEFAULT NULL;"
                       "        t_middle_name TEXT DEFAULT NULL;"
                       "        t_last_name TEXT DEFAULT NULL;"
                       "    BEGIN"
                       "        SELECT internalid, publicid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE publicid = v_instance_uuid)))"
                       "            INTO"
                       "                t_internalid, t_patient_uuid;"
                       ""
                       "        SELECT"
                       "            split_part(v_full_name,' ',1),"
                       "            CASE"
                       "                WHEN split_part(v_full_name,' ',3) = '' THEN NULL ELSE split_part(v_full_name,' ',2) END,"
                       "            CASE"
                       "                WHEN split_part(v_full_name,' ',2) = '' THEN NULL"
                       "                WHEN split_part(v_full_name,' ',3) = '' THEN split_part(v_full_name,' ',2)"
                       "                ELSE split_part(v_full_name,' ',3)"
                       "                END"
                       "        INTO"
                       "            t_first_name, t_middle_name, t_last_name;"
                       ""
                       "        INSERT INTO"
                       "            crosswalk(internalid, publicid, patient_id, full_name, first_name, middle_name, last_name, dob, instances)"
                       "        VALUES"
                       "            (t_internalid, t_patient_uuid, v_patient_id, v_full_name, t_first_name, t_middle_name, t_last_name, v_dob, ARRAY[v_instance_uuid])"
                       "            ON CONFLICT"
                       "                (publicid, patient_id, full_name, dob)"
                       "            DO UPDATE"
                       "                SET instances = array_append(crosswalk.instances, v_instance_uuid);"
                       "    END;"
                       "$insert_info_crosswalk$ LANGUAGE plpgsql;"
                       );
                w.exec(""
                       "CREATE OR REPLACE FUNCTION detect_phi_mismatch() RETURNS trigger AS $detect_phi_mismatch$"
                       "	DECLARE"
                       "		dob_fullname_internalid BIGINT;"
                       "		dob_fullname_publicid TEXT;"
                       "		dob_lastname_internalid BIGINT;"
                       "		dob_lastname_publicid TEXT;"
                       "	BEGIN"
                       "        SELECT internalid, publicid FROM crosswalk"
                       "        INTO"
                       "            dob_fullname_internalid, dob_fullname_publicid"
                       "        WHERE"
                       "            internalid <> NEW.internalid AND"
                       "            dob = NEW.dob AND"
                       "            lower(full_name) = lower(NEW.full_name)"
                       "        ORDER BY internalid ASC"
                       "        LIMIT 1;"
                       ""
                       "        /* Detect case 5 and 6 */"
                       "        IF dob_fullname_internalid IS NOT NULL THEN"
                       "            INSERT INTO phi_mismatch VALUES(NEW.internalid, NEW.publicid, dob_fullname_internalid, dob_fullname_publicid);"
                       "            RAISE NOTICE 'PHI mismatch detected - DOB & Full Name matched: parent_internalid: %, parent_publicid: %', dob_fullname_internalid, dob_fullname_publicid;"
                       "        ELSE"
                       "            SELECT internalid, publicid"
                       "            INTO"
                       "                dob_lastname_internalid, dob_lastname_publicid"
                       "            FROM crosswalk"
                       "            WHERE"
                       "                  internalid <> NEW.internalid AND"
                       "                  dob = NEW.dob AND"
                       "                  lower(last_name) = lower(NEW.last_name)"
                       "            ORDER BY internalid ASC"
                       "            LIMIT 1;"
                       ""
                       "            IF dob_lastname_internalid IS NOT NULL THEN"
                       "                INSERT INTO phi_mismatch VALUES(NEW.internalid, NEW.publicid, dob_lastname_internalid, dob_lastname_publicid);"
                       "                RAISE NOTICE 'PHI mismatch detected - DOB & Last Name matched: parent_internalid: %, parent_publicid: %', dob_lastname_internalid, dob_lastname_publicid;"
                       "            END IF;"
                       "        END IF;"
                       "		RETURN NULL;"
                       "	END;"
                       "$detect_phi_mismatch$ LANGUAGE plpgsql;"
                );

                // Create triggers
                w.exec(""
                       "CREATE OR REPLACE TRIGGER new_patient AFTER UPDATE"
                       "    ON crosswalk"
                       "    FOR EACH ROW"
                       "    EXECUTE PROCEDURE detect_phi_mismatch();"
                      );

                w.commit();
                con.close();
                return_value = true;
            }
        });
        return return_value;
        //con->prepare("my_simp","SELECT * FROM attachedfiles");
    } catch (const std::exception &e) {
        DEBUG_LOG(PLUGIN_ERRORS, "Something went wrong during DB initialization");
        DEBUG_LOG(PLUGIN_ERRORS, e.what());
    }
    return false;
}
