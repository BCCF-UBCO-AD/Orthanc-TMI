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
            "\n"
            "CREATE TABLE IF NOT EXISTS public.patient_list (\n"
            "    id INT DEFAULT nextval('id_sequence'::regclass) NOT NULL,\n"
            "    uuid TEXT NOT NULL,\n"
            "    PRIMARY KEY (id)\n"
            ");\n"
            "\n"
            "DROP FUNCTION add_patient() CASCADE;\n"
            "CREATE FUNCTION add_patient()\n"
            "   RETURNS TRIGGER\n"
            "   LANGUAGE PLPGSQL\n"
            "AS $$\n"
            "BEGIN\n"
            "   if CAST(NEW.taggroup as text) like CAST(16 as text)\n"
            "           and CAST(NEW.tagelement as text) like CAST(32 as text) then\n"
            "       if not exists(select uuid from public.patient_list where uuid like NEW.value) then\n"
            "           insert into public.patient_list(id, uuid)\n"
            "               values (nextval('id_sequence'), NEW.value) on conflict do nothing;\n"
            "       end if;\n"
            "   end if;\n"
            "   RETURN NEW;\n"
            "END;\n"
            "$$;\n"
            "\n"
            "CREATE TRIGGER catch_new_patient\n"
            "   AFTER INSERT OR UPDATE ON public.dicomidentifiers\n"
            "   FOR EACH ROW\n"
            "   EXECUTE PROCEDURE add_patient();\n"
            "\n"
            "CREATE TABLE IF NOT EXISTS public.dicom_duplicates (\n"
            "    id VARCHAR PRIMARY KEY,\n"
            "    parent_id VARCHAR,\n"
            "    FOREIGN KEY (id) REFERENCES public.patient_list(id),\n"
            "    FOREIGN KEY (parent_id) REFERENCES public.patient_list(id)\n"
            ");\n"
            );
    w.commit();
}
