cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")

# project variables
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
project(Orthanc-TMI)
set(PLUGIN_NAME DataAnonymizer)
set(TEST_TARGET all-tests)

# dependencies
file(GLOB_RECURSE PLUGIN_SOURCES LIST_DIRECTORIES false src/*)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false test/*)
add_subdirectory(extern/googletest)
add_subdirectory(extern/libpqxx)
#add_subdirectory(extern/json)

# plugin target
include_directories(include)
include_directories(extern/json/single_include/)
add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})
add_custom_command(
        TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.so
        ${PROJECT_DIR}/docker/orthanc/plugins/${PLUGIN_NAME}.so)

# test target
#include_directories(${gmock_build_include_dirs})

add_executable(${TEST_TARGET} ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TEST_TARGET} PUBLIC gtest pqxx)

add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
