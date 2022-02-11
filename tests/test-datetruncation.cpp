#include "date-truncation.h"
#include "common.h"
#include "gtest/gtest.h"

TEST(datetimetests, datetruncation){
    std::string defvalue = "19990323";
    const char* tcase0 = "YYYYMMDD";
    const char* tcase1 = "YYYYMM10";
    const char* tcase2 = "YYYY06DD";
    const char* tcase3 = "2000MMDD";

    std::string value;
    value = DateTruncation(defvalue, tcase0);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990323");
    value = DateTruncation(defvalue, tcase1);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990310");
    value = DateTruncation(defvalue, tcase2);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990623");
    value = DateTruncation(defvalue, tcase3);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "20000323");
    value = DateTruncation(defvalue);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990323");
}

