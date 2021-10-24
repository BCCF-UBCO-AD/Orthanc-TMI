cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")

# project variables
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
project(Orthanc-TMI)
set(PLUGIN_NAME DataAnonymizer)
set(TESTS ${CMAKE_PROJECT_NAME}-tests)

# dependencies
file(GLOB_RECURSE PLUGIN_SOURCES LIST_DIRECTORIES true src/*)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES true test/*)
add_subdirectory(lib/googletest)

# plugin target
include_directories(include)
add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})
#add_custom_command(
#        TARGET ${PLUGIN_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.so
#        ${PROJECT_DIR}/docker/orthanc/plugins/${PLUGIN_NAME}.so)

# test target
include_directories(lib/googletest/googlemock/include)
include_directories(lib/googletest/googletest/include)
add_executable(${TESTS} ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TESTS} PUBLIC gtest)

add_test(NAME ${TESTS} COMMAND ${TESTS})