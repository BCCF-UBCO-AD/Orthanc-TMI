#include <gtest/gtest.h>

//appears to be the form of a basic unit test
// https://github.com/google/googletest/blob/master/googletest/samples/sample1_unittest.cc
// https://google.github.io/googletest/primer.html
TEST(Placeholder, Example) {
    ASSERT_STREQ("Hello","Hello");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}