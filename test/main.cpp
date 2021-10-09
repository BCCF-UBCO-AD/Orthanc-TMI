#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

//appears to be the form of a basic unit test
// https://github.com/google/googletest/blob/master/googletest/samples/sample1_unittest.cc
// https://google.github.io/googletest/primer.html
TEST(libraries, json_test) {
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
}

TEST(Placeholder, Example) {
    ASSERT_STREQ("Hello","Hello");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}