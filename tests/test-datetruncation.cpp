#include <gtest/gtest.h>
#include <date-truncation.h>
#include <fstream>
#include "core.h"
#include "plugin-configure.h"

static nlm::json config;
static std::ifstream file("../../docker/orthanc/orthanc.json");

TEST(datetrunc, trunc_test){
    file >> config;
    std::string value = DateTruncation(config, "19990323");
    ASSERT_STREQ(value.c_str(), "19990323");
}

