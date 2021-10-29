cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
set(BSL ${BUILD_SHARED_LIBS})

# project variables
project(Orthanc-TMI)
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# target names
set(PLUGIN_TARGET data-anonymizer)
set(TEST_TARGET all-tests)

# dependencies
file(GLOB_RECURSE PLUGIN_SOURCES LIST_DIRECTORIES false src/*)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false tests/*)
add_subdirectory(extern/googletest)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(extern/libpqxx)
set(BUILD_SHARED_LIBS ${BSL})

# include directories
include_directories(include)
include_directories(extern/nlohmann-json/single_include/)

# test target
add_executable(${TEST_TARGET} ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TEST_TARGET} PUBLIC gtest pqxx)
add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

# plugin target
add_library(${PLUGIN_TARGET} SHARED ${PLUGIN_SOURCES})
target_link_libraries(${PLUGIN_TARGET} pqxx)
add_custom_command(
        TARGET ${PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_BINARY_DIR}/bin/${PLUGIN_TARGET}.so
        ${PROJECT_DIR}/docker/orthanc/plugins/${PLUGIN_TARGET}.so)
