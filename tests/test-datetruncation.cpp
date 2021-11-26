#include <gtest/gtest.h>
#include <date-truncation.h>
#include <fstream>
#include "common.h"
#include "core.h"


static nlm::json config;
fs::path config_path(GetProjRoot().string() + "/docker/orthanc/orthanc.json");
static std::ifstream file(config_path);

TEST(datetrunc, trunc_test){
    file >> config;
    std::string value = DateTruncation(config, "19990323");
    ASSERT_STREQ(value.c_str(), "19990410");
}

