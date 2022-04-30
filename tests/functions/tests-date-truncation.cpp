#include "../common.h"
#include <gtest/gtest.h>

extern std::string TruncateDate(std::string date, std::string format);

TEST(functions, datetruncation){
    std::cout << "Unit Test: functions datetruncation" << std::endl;
    std::string defvalue = "19990323";
    const char* tcase0 = "YYYYMMDD";
    const char* tcase1 = "YYYY0101";
    const char* tcase2 = "YYYYMM10";
    const char* tcase3 = "YYYY06DD";
    const char* tcase4 = "2000MMDD";

    std::string value;
    value = TruncateDate(defvalue, tcase0);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990323");

    value = TruncateDate(defvalue, tcase1);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990101");

    value = TruncateDate(defvalue, tcase2);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990310");

    value = TruncateDate(defvalue, tcase3);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990623");

    value = TruncateDate(defvalue, tcase4);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "20000323");
    std::cout << "Test Complete.\n" << std::endl;
}

