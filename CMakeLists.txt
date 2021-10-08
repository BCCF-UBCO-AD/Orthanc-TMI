cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")

# project variables
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
project(Orthanc-TMI)
set(PLUGIN_NAME DataAnonymizer)
set(TESTS ${CMAKE_PROJECT_NAME}-tests)

# dependencies
file(GLOB_RECURSE PLUGIN_SOURCES LIST_DIRECTORIES true src/*)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES true test/*)
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
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PLUGIN_NAME}.so
        ${PROJECT_DIR}/docker/plugins/${PLUGIN_NAME}.so)

# test target
#include_directories(${gmock_build_include_dirs})

add_executable(${TESTS} ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TESTS} PUBLIC gtest pqxx)

add_test(NAME ${TESTS} COMMAND ${TESTS})
