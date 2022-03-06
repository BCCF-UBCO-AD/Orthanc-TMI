#include <core.h>
#include <db-interface.h>
#include <pqxx/except.hxx>
#include <job-queue.h>
#include <thread>
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
    if(!con || !con->is_open()) {
        delete con;
        try {
            char buffer[1024];
            DEBUG_LOG(DEBUG_2, "Preparing connection string");
            sprintf(buffer, "postgresql://%s:%s@%s:%hu/%s", username.c_str(), password.c_str(), host.c_str(), port, database.c_str());
            DEBUG_LOG(DEBUG_2, buffer);
            con = new pqxx::connection(buffer); // not using a pointer causes the function to throw an exception when invoked.. doesn't make any sense.. but let's just leave it as is

            JobQueue::GetInstance().AddJob([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                initialize();
            });
        } catch (const std::exception &e) {
            DEBUG_LOG(PLUGIN_ERRORS, "Failed to connect to database.")
            DEBUG_LOG(PLUGIN_ERRORS, e.what());
        }
    }
}

void DBInterface::initialize() {
    if(con && con->is_open()){
        try {
            pqxx::work w(*con);

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
            // Prepared Statements
            con->prepare(
                    "UpdateChecksum",
                    "UPDATE attachedfiles "
                    "SET uncompressedsize = $1, compressedsize = $1, uncompressedhash = $2, compressedhash = $2 "
                    "WHERE uuid = $3;"
                        );
        }   catch (const std::exception &e) {
            DEBUG_LOG(PLUGIN_ERRORS, "Something went wrong during DB initialization");
            DEBUG_LOG(PLUGIN_ERRORS, e.what());
        }
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

void DBInterface::UpdateChecksum(std::string uuid, std::string hash, int64_t size) {
    try {
        if (con && con->is_open()) {
            pqxx::work w(*con);
            w.exec_prepared("UpdateChecksum", size, size, hash, hash, uuid);
            w.commit();
        }
    } catch (const std::exception &e){
        DEBUG_LOG(PLUGIN_ERRORS, "UpdateChecksum() encountered an exception");
        DEBUG_LOG(PLUGIN_ERRORS, e.what());
    }
}
