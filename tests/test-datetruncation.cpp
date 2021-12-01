#include "date-truncation.h"
#include "gtest/gtest.h"

TEST(datetimetests, datetruncation){
    std::string defvalue = "19990323";
    nlm::json tcase0 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","MM","DD"]}})"_json;
    nlm::json tcase1 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","MM","10"]}})"_json;
    nlm::json tcase2 = R"({"Dicom-DateTruncation": {"dateformat": ["YYYY","06","DD"]}})"_json;
    nlm::json tcase3 = R"({"Dicom-DateTruncation": {"dateformat": ["2000","MM","DD"]}})"_json;
    nlm::json tcase4 = R"({"Dicom-DateTruncation": {"dateformat": ["1998","11","30"]}})"_json;

    ASSERT_TRUE(DateTruncation(tcase0, defvalue).compare("19990323"));
    ASSERT_TRUE(DateTruncation(tcase1, defvalue).compare("19990310"));
    ASSERT_TRUE(DateTruncation(tcase2, defvalue).compare("19990623"));
    ASSERT_TRUE(DateTruncation(tcase3, defvalue).compare("20000323"));
    ASSERT_TRUE(DateTruncation(tcase4, defvalue).compare("19981130"));
}

