#include "date-truncation.h"
#include "common.h"
#include "gtest/gtest.h"

TEST(datetimetests, datetruncation){
    std::string defvalue = "19990323";
    nlm::json tcase0 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","MM","DD"]}})"_json;
    nlm::json tcase1 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","MM","10"]}})"_json;
    nlm::json tcase2 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","06","DD"]}})"_json;
    nlm::json tcase3 = R"({"Dicom-DateTruncation": {"dateformat": ["2000","MM","DD"]}})"_json;

    std::string value;
    value = DateTruncation(tcase0, defvalue);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990323");
    value = DateTruncation(tcase1, defvalue);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990310");
    value = DateTruncation(tcase2, defvalue);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "19990623");
    value = DateTruncation(tcase3, defvalue);
    std::cout << value << std::endl;
    ASSERT_EQ(value, "20000323");

}

