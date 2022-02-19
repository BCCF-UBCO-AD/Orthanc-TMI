#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <pqxx/field.hxx>
#include <db-interface.h>
#include "common.h"

TEST(postgrsql, basicsqltest){
    pqxx::connection c{"postgresql://postgres:example@localhost:5432"};
    pqxx::work txn{c};
    txn.exec0("CREATE TABLE IF NOT EXISTS public.team (id int, name varchar, age int, title varchar(4));");
    pqxx::result r{txn.exec("SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_schema='public' AND table_name='team');")};
    pqxx::row row = r[0];
    pqxx::field field  = row[0];
    ASSERT_STREQ(field.c_str(), "t");

    txn.exec("INSERT INTO public.team  (id, name, age, title)VALUES (0,'jack',25,'QA') RETURNING id;");
    pqxx::result s{txn.exec("UPDATE public.team SET id = 10 RETURNING id;")};
    row = s[0];
    field = row[0];
    ASSERT_TRUE(field.get<int>() == 10);

    txn.exec0("DROP TABLE IF EXISTS public.team;");
    pqxx::result n{txn.exec("SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_schema='public' AND table_name='team');")};
    row = n[0];
    field  = row[0];
    txn.commit();
    ASSERT_STREQ(field.c_str(), "f");
}

