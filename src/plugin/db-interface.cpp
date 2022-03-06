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

void DBInterface::Initialize() {
    try {
        static std::once_flag flag;
        std::call_once(flag, []() {
            pqxx::connection con(PluginConfigurer::GetDBConnectionInfo());
            if (con.is_open()) {
                pqxx::work w(con);
                // todo: maybe load this SQL code from file? or define the strings somewhere else? a sql file would seem ideal but then packaging and all that
                // Alter reousrces to add unique constraint
                w.exec(""
                       "DO $$"
                       "	BEGIN"
                       "		IF NOT EXISTS (SELECT constraint_name FROM information_schema.constraint_column_usage "
                       "					   WHERE table_name = 'resources' "
                       "					   AND constraint_name = 'unique_publicid') THEN"
                       "			ALTER TABLE resources ADD CONSTRAINT unique_publicid UNIQUE(publicid);"
                       "		END IF;"
                       "	END;"
                       "$$ language 'plpgsql';"
                      );

                // Create crosswalk table
                w.exec(""
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
                       "	FOREIGN KEY (internalid) REFERENCES resources(internalid) ON DELETE CASCADE,"
                       "	FOREIGN KEY (publicid) REFERENCES resources(publicid) ON DELETE CASCADE"
                       ");"
                      );
                w.exec(""
                       "CREATE TABLE IF NOT EXISTS phi_mismatch ("
                       "	internalid BIGINT NOT NULL UNIQUE,"
                       "	publicid CHARACTER VARYING(64) NOT NULL UNIQUE,"
                       "	parent_internalid BIGINT NOT NULL,"
                       "	parent_publicid CHARACTER VARYING(64) NOT NULL,"
                       "	PRIMARY KEY (internalid),"
                       "	FOREIGN KEY (internalid) REFERENCES resources(internalid) ON DELETE CASCADE,"
                       "	FOREIGN KEY (publicid) REFERENCES resources(publicid) ON DELETE CASCADE,"
                       "	FOREIGN KEY (parent_internalid) REFERENCES resources(internalid) ON DELETE CASCADE,"
                       "	FOREIGN KEY (parent_publicid) REFERENCES resources(publicid) ON DELETE CASCADE"
                       ");"
                      );

                // Create indexes
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
                w.exec(""
                       "CREATE OR REPLACE FUNCTION detect_phi_mismatch() RETURNS trigger AS $detect_phi_mismatch$"
                       "	DECLARE"
                       "		dob_fullname_internalid BIGINT;"
                       "		dob_fullname_publicid TEXT;"
                       "		dob_lastname_internalid BIGINT;"
                       "		dob_lastname_publicid TEXT;"
                       "	BEGIN"
                       "		/* Wait until all info is in crosswalk */"
                       "		IF NEW.internalid IS NOT NULL AND NEW.patient_id IS NOT NULL AND NEW.full_name IS NOT NULL AND NEW.dob IS NOT NULL THEN"
                       "			SELECT internalid, publicid INTO dob_fullname_internalid, dob_fullname_publicid FROM crosswalk WHERE internalid <> NEW.internalid AND dob = NEW.dob AND lower(full_name) = lower(NEW.full_name) ORDER BY internalid ASC LIMIT 1;"
                       "			IF dob_fullname_internalid IS NOT NULL THEN"
                       "				INSERT INTO phi_mismatch VALUES(NEW.internalid, NEW.publicid, dob_fullname_internalid, dob_fullname_publicid);"
                       "				RAISE NOTICE 'PHI mismatch detected - DOB & Full Name matched: parent_internalid: %, parent_publicid: %', dob_fullname_internalid, dob_fullname_publicid;"
                       "			ELSE"
                       "				SELECT internalid, publicid INTO dob_lastname_internalid, dob_lastname_publicid FROM crosswalk WHERE internalid <> NEW.internalid AND dob = NEW.dob AND lower(last_name) = lower(NEW.last_name) ORDER BY internalid ASC LIMIT 1;"
                       "				IF dob_lastname_internalid IS NOT NULL THEN"
                       "					INSERT INTO phi_mismatch VALUES(NEW.internalid, NEW.publicid, dob_lastname_internalid, dob_lastname_publicid);"
                       "					RAISE NOTICE 'PHI mismatch detected - DOB & Last Name matched: parent_internalid: %, parent_publicid: %', dob_lastname_internalid, dob_lastname_publicid;"
                       "				END IF;"
                       "			END IF;			"
                       "		END IF;"
                       "		RETURN NULL;"
                       "	END;"
                       "$detect_phi_mismatch$ LANGUAGE plpgsql;"
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
                w.exec(""
                       "CREATE OR REPLACE TRIGGER new_patient AFTER UPDATE"
                       "	ON crosswalk"
                       "	FOR EACH ROW"
                       "	EXECUTE PROCEDURE detect_phi_mismatch();"
                      );
                w.commit();
                con.close();
            }
        });
        //con->prepare("my_simp","SELECT * FROM attachedfiles");
    } catch (const std::exception &e) {
        DEBUG_LOG(PLUGIN_ERRORS, "Something went wrong during DB initialization");
        DEBUG_LOG(PLUGIN_ERRORS, e.what());
    }
}
