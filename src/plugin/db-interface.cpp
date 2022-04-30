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

void DBInterface::InsertCrosswalk(std::string file_uuid, std::string patient_id, std::string full_name, std::string dob) {
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
            w.exec_prepared("InsertCrosswalk", file_uuid, patient_id, full_name, dob);
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
                       "    files TEXT[],"
                       "	PRIMARY KEY (id),"
                       "    UNIQUE (internalid, publicid, patient_id, full_name, dob)"
                       ");"
                      );

                // Create mismatch table
                w.exec(""
                       "CREATE TABLE IF NOT EXISTS phi_mismatch("
                       "    id SERIAL NOT NULL,"
                       "    mismatch_type INT NOT NULL,"
                       "    crosswalk_id BIGINT NOT NULL,"
                       "    internalid BIGINT NOT NULL,"
                       "    publicid CHARACTER VARYING(64) NOT NULL,"
                       "    patient_id TEXT NOT NULL,"
                       "    parent_crosswalk_id BIGINT NOT NULL,"
                       "    parent_internalid BIGINT NOT NULL,"
                       "    parent_publicid CHARACTER VARYING(64) NOT NULL,"
                       "    parent_patient_id TEXT NOT NULL,"
                       "    PRIMARY KEY (id)"
                       ");"
                      );

                // Create indexes
                w.exec("CREATE INDEX IF NOT EXISTS i_internalid ON crosswalk (internalid);");
                w.exec("CREATE INDEX IF NOT EXISTS i_publicid ON crosswalk (publicid);");
                w.exec("CREATE INDEX IF NOT EXISTS i_patient_id ON crosswalk (lower(patient_id));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_full_name ON crosswalk (lower(full_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_first_name ON crosswalk (lower(first_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_middle_name ON crosswalk (lower(middle_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_lower_last_name ON crosswalk (lower(last_name));");
                w.exec("CREATE INDEX IF NOT EXISTS i_dob ON crosswalk (lower(dob));");

                // Create function for adding info to crosswalk table
                w.exec(""
                       "CREATE OR REPLACE FUNCTION insert_info_crosswalk(v_file_uuid TEXT, v_patient_id TEXT, v_full_name TEXT, v_dob TEXT) RETURNS void AS $insert_info_crosswalk$"
                       "    DECLARE"
                       "        t_internalid BIGINT DEFAULT NULL;"
                       "        t_patient_uuid TEXT DEFAULT NULL;"
                       "        t_first_name TEXT DEFAULT NULL;"
                       "        t_middle_name TEXT DEFAULT NULL;"
                       "        t_last_name TEXT DEFAULT NULL;"
                       "    BEGIN"
                       "        v_file_uuid := BTRIM(v_file_uuid);"
                       "        v_patient_id := BTRIM(v_patient_id);"
                       "        v_full_name := BTRIM(v_full_name);"
                       "        v_dob := BTRIM(v_dob);"
                       ""
                       "        SELECT internalid, publicid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE internalid ="
                       "            (SELECT parentid FROM resources WHERE internalid ="
                       "            (SELECT id FROM attachedfiles WHERE uuid = v_file_uuid))))"
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
                       "            crosswalk(internalid, publicid, patient_id, full_name, first_name, middle_name, last_name, dob, files)"
                       "        VALUES"
                       "            (t_internalid, t_patient_uuid, v_patient_id, v_full_name, t_first_name, t_middle_name, t_last_name, v_dob, ARRAY[v_file_uuid])"
                       "            ON CONFLICT"
                       "                (internalid, publicid, patient_id, full_name, dob)"
                       "            DO UPDATE"
                       "                SET files = array_append(crosswalk.files, v_file_uuid);"
                       "    END;"
                       "$insert_info_crosswalk$ LANGUAGE plpgsql;"
                       );

                // Create procedure for phi mismath detection
                w.exec(""
                       "CREATE OR REPLACE FUNCTION detect_phi_mismatch() RETURNS trigger AS $detect_phi_mismatch$"
                       "	DECLARE"
                       "        /* Patient ID does not match */"
                       "		case5 RECORD DEFAULT NULL; /* Same DOB, Full name */"
                       "		case6 RECORD DEFAULT NULL; /* Same DOB, Last name  */"
                       ""
                       "        /* Patient ID matches */"
                       "        case4 RECORD DEFAULT NULL; /* Same Full name - Different DOB */"
                       "        case7 RECORD DEFAULT NULL; /* Same First name, Last name, DOB - Different Middle name */"
                       "        case3 RECORD DEFAULT NULL; /* Same First name, DOB - Different Last name */"
                       "        case2 RECORD DEFAULT NULL; /* Same Last name, DOB - Different First name */"
                       "	BEGIN"
                       "	    RAISE NOTICE 'Mismatch detection started.';"
                       ""
                       "        SELECT"
                       "            *"
                       "        INTO"
                       "            case5"
                       "        FROM crosswalk"
                       "        WHERE"
                       "            patient_id <> NEW.patient_id AND"
                       "            internalid <> NEW.internalid AND"
                       "            lower(dob) IS NOT DISTINCT FROM lower(NEW.dob) AND"
                       "            lower(full_name) IS NOT DISTINCT FROM lower(NEW.full_name)"
                       "        ORDER BY internalid ASC"
                       "        LIMIT 1;"
                       ""
                       "        /* Detect case 5 and 6 */"
                       "        IF case5.id IS NOT NULL THEN"
                       "            INSERT INTO"
                       "                phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "            VALUES"
                       "                (5, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case5.id, case5.internalid, case5.publicid, case5.patient_id);"
                       "            RAISE NOTICE 'PHI mismatch detected - Case #5: parent_internalid: %, parent_publicid: %', case5.internalid, case5.publicid;"
                       "        ELSE"
                       "            SELECT"
                       "                *"
                       "            INTO"
                       "                case6"
                       "            FROM crosswalk"
                       "            WHERE"
                       "                patient_id <> NEW.patient_id AND"
                       "                internalid <> NEW.internalid AND"
                       "                lower(dob) IS NOT DISTINCT FROM lower(NEW.dob) AND"
                       "                lower(last_name) IS NOT DISTINCT FROM lower(NEW.last_name)"
                       "            ORDER BY internalid ASC"
                       "            LIMIT 1;"
                       ""
                       "            IF case6.id IS NOT NULL THEN"
                       "                INSERT INTO"
                       "                    phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "                VALUES"
                       "                    (6, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case6.id, case6.internalid, case6.publicid, case6.patient_id);"
                       "                RAISE NOTICE 'PHI mismatch detected - Case #6: parent_internalid: %, parent_publicid: %', case6.internalid, case6.publicid;"
                       "            END IF;"
                       "        END IF;"
                       ""
                       "        /* case 4 */"
                       "        SELECT"
                       "            *"
                       "        INTO"
                       "            case4"
                       "        FROM crosswalk"
                       "        WHERE"
                       "            patient_id = NEW.patient_id AND"
                       "            internalid = NEW.internalid AND"
                       "            lower(full_name) IS NOT DISTINCT FROM lower(NEW.full_name) AND"
                       "            lower(dob) IS DISTINCT FROM lower(NEW.dob)"
                       "        ORDER BY id ASC"
                       "            LIMIT 1;"
                       ""
                       "        IF case4.id IS NOT NULL THEN"
                       "            INSERT INTO"
                       "                phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "            VALUES"
                       "                (4, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case4.id, case4.internalid, case4.publicid, case4.patient_id);"
                       "            RAISE NOTICE 'PHI mismatch detected - Case #4: parent_internalid: %, parent_publicid: %', case4.internalid, case4.publicid;"
                       "        END IF;"
                       ""
                       "        /* Case 7 */"
                       "        SELECT"
                       "            *"
                       "        INTO"
                       "            case7"
                       "        FROM crosswalk"
                       "        WHERE"
                       "            patient_id = NEW.patient_id AND"
                       "            internalid = NEW.internalid AND"
                       "            lower(first_name) IS NOT DISTINCT FROM lower(NEW.first_name) AND"
                       "            lower(last_name) IS NOT DISTINCT FROM lower(NEW.last_name) AND"
                       "            lower(dob) IS NOT DISTINCT FROM lower(NEW.dob) AND"
                       "            lower(middle_name) IS DISTINCT FROM lower(NEW.middle_name)"
                       "        ORDER BY id ASC"
                       "        LIMIT 1;"
                       ""
                       "        IF case7.id IS NOT NULL THEN"
                       "            INSERT INTO"
                       "                phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "            VALUES"
                       "                (7, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case7.id, case7.internalid, case7.publicid, case7.patient_id);"
                       "            RAISE NOTICE 'PHI mismatch detected - Case #7: parent_internalid: %, parent_publicid: %', case7.internalid, case7.publicid;"
                       "        END IF;"
                       ""
                       "        /* Case 3 */"
                       "        SELECT"
                       "            *"
                       "        INTO"
                       "            case3"
                       "        FROM crosswalk"
                       "        WHERE"
                       "            patient_id = NEW.patient_id AND"
                       "            internalid = NEW.internalid AND"
                       "            lower(first_name) IS NOT DISTINCT FROM lower(NEW.first_name) AND"
                       "            lower(dob) IS NOT DISTINCT FROM lower(NEW.dob) AND"
                       "            lower(last_name) IS DISTINCT FROM lower(NEW.last_name)"
                       "        ORDER BY id ASC"
                       "            LIMIT 1;"
                       ""
                       "        IF case3.id IS NOT NULL THEN"
                       "            INSERT INTO"
                       "                phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "            VALUES"
                       "                (3, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case3.id, case3.internalid, case3.publicid, case3.patient_id);"
                       "            RAISE NOTICE 'PHI mismatch detected - Case #3: parent_internalid: %, parent_publicid: %', case3.internalid, case3.publicid;"
                       "        END IF;"
                       ""
                       "        /* Case 2 */"
                       "        SELECT"
                       "            *"
                       "        INTO"
                       "            case2"
                       "        FROM crosswalk"
                       "        WHERE"
                       "            patient_id = NEW.patient_id AND"
                       "            internalid = NEW.internalid AND"
                       "            lower(last_name) IS NOT DISTINCT FROM lower(NEW.last_name) AND"
                       "            lower(dob) IS NOT DISTINCT FROM lower(NEW.dob) AND"
                       "            lower(first_name) IS DISTINCT FROM lower(NEW.first_name)"
                       "        ORDER BY id ASC"
                       "            LIMIT 1;"
                       ""
                       "        IF case2.id IS NOT NULL THEN"
                       "            INSERT INTO"
                       "                phi_mismatch(mismatch_type, crosswalk_id, internalid, publicid, patient_id, parent_crosswalk_id, parent_internalid, parent_publicid, parent_patient_id)"
                       "            VALUES"
                       "                (2, NEW.id, NEW.internalid, NEW.publicid, NEW.patient_id, case2.id, case2.internalid, case2.publicid, case2.patient_id);"
                       "            RAISE NOTICE 'PHI mismatch detected - Case #2: parent_internalid: %, parent_publicid: %', case2.internalid, case2.publicid;"
                       "        END IF;"
                       ""
                       "        RAISE NOTICE 'Mismatch detection ended.';"
                       "    RETURN NULL;"
                       "	END;"
                       "$detect_phi_mismatch$ LANGUAGE plpgsql;"
                );

                // Create triggers
                w.exec(""
                       "CREATE OR REPLACE TRIGGER new_patient AFTER INSERT"
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
