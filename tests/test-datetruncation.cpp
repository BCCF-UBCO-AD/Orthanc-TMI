#include <gtest/gtest.h>
#include <date-truncation.h>
#include "core.h"
#include "plugin-configure.h"

nlm::json config;

TEST(datetrunc, trunc_test){
    config = nlm::json::parse(OrthancPluginGetConfiguration(globals::context));
    std::string value = DateTruncation(config, "19990323");
    ASSERT_STREQ(value.c_str(), "19990323");
}

