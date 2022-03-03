#include "../common.h"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <db-interface.h>

//appears to be the form of a basic unit test
// https://github.com/google/googletest/blob/master/googletest/samples/sample1_unittest.cc
// https://google.github.io/googletest/primer.html
TEST(libraries, json) {
    std::cout << "Unit Test: libraries json" << std::endl;
    using json = nlohmann::json;
    // create an empty structure (null)
    json j;

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;
    // add a Boolean that is stored as bool
    j["happy"] = true;
    // add a string that is stored as std::string
    j["name"] = "Niels";
    // add another null object by passing nullptr
    j["nothing"] = nullptr;
    // add an object inside the object
    j["answer"]["everything"] = 42;
    // add an array that is stored as std::vector (using an initializer list)
    j["list"] = {1, 0, 2};
    // add another object (using an initializer list of pairs)
    j["object"] = {{"currency", "USD"},
    {"value",    42.99}};

    // instead, you could also write (which looks very similar to the JSON above)
    json j2 = {
            {"pi",      3.141},
            {"happy",   true},
            {"name",    "Niels"},
            {"nothing", nullptr},
            {"answer",  {
                                {"everything", 42}
                        }},
            {"list",    {       1, 0, 2}},
            {"object",  {
                                {"currency",   "USD"},
                                   {"value", 42.99}
                        }}
    };
    ASSERT_EQ(j,j2);
    j["pi"]=3.1;
    //ASSERT_EQ(j,j2);
    ASSERT_NE(j,j2);
    std::cout << "Test Complete.\n" << std::endl;
}

TEST(libraries, pqxx){
    std::cout << "Unit Test: libraries pqxx" << std::endl;
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
    //pqxx::connection c("postgresql://postgres:example@localhost:5432");
    //pqxx::work w(c);
    DBInterface::GetInstance().Connect("orthanc", "localhost", 5432, "postgres", "example");
    ASSERT_TRUE(DBInterface::GetInstance().IsOpen());
    std::cout << "Test Complete.\n" << std::endl;
}