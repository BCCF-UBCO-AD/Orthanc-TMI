cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")

# project variables
project(Orthanc-TMI)
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# target names
set(PLUGIN_TARGET DataAnonymizer)
set(TEST_TARGET all-tests)

# dependencies
file(GLOB_RECURSE PLUGIN_SOURCES LIST_DIRECTORIES false src/*)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false tests/*)
add_subdirectory(extern/googletest)
# LIBPQ
find_package(PostgreSQL)
if( NOT PostgreSQL_FOUND )
    message("## Couldn't find PostgreSQL installed on system.")
    # directories
    set(LIBPQ_INSTALL ${PROJECT_BINARY_DIR}/libpq-install/)
    set(POSTGRESQL_DIR ${PROJECT_DIR}/extern/postgres/)
    # configure make build & its install directory
    message("## configuring extern/postgres")
    execute_process(COMMAND configure --prefix=${LIBPQ_INSTALL}
            WORKING_DIRECTORY ${POSTGRESQL_DIR})
    # build & install
    message("## building extern/postgres")
    execute_process(COMMAND make install
            WORKING_DIRECTORY ${POSTGRESQL_DIR})
    # libpqxx variable for where to find install
    message("## telling cmake/libpqxx where to find PostgreSQL")
    set(PostgreSQL_ROOT ${LIBPQ_INSTALL})
endif()
add_subdirectory(extern/libpqxx)
#add_subdirectory(extern/json)

# plugin target
include_directories(include)
include_directories(extern/nlohmann-json/single_include/)
add_library(${PLUGIN_TARGET} SHARED ${PLUGIN_SOURCES})
add_custom_command(
        TARGET ${PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_BINARY_DIR}/bin/${PLUGIN_TARGET}.so
        ${PROJECT_DIR}/docker/orthanc/plugins/${PLUGIN_TARGET}.so)

# test target
add_executable(${TEST_TARGET} ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TEST_TARGET} PUBLIC gtest pqxx)

add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
