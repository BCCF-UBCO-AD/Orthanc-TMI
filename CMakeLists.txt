cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-fsanitize=address,undefined")
    add_link_options("-fsanitize=address,undefined")
endif()
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
list(FILTER TEST_SOURCES EXCLUDE REGEX .*main.cpp)
message("Plugin sources: ${PLUGIN_SOURCES}")
message("Common test sources: ${TEST_SOURCES}")


add_subdirectory(extern/googletest EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(extern/libpqxx EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ${BSL})

# include directories
include_directories(include)
include_directories(include/dicom)
include_directories(include/data)
include_directories(extern/nlohmann-json/single_include/)

# test targets
add_executable(${TEST_TARGET} tests/main.cpp ${TEST_SOURCES} ${PLUGIN_SOURCES})
target_link_libraries(${TEST_TARGET} PUBLIC gtest pqxx)
add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

# plugin target
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
add_library(${PLUGIN_TARGET} SHARED ${PLUGIN_SOURCES})
target_link_libraries(${PLUGIN_TARGET} PUBLIC pqxx)
add_custom_command(
        TARGET ${PLUGIN_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_BINARY_DIR}/bin/${PLUGIN_TARGET}.so
        ${PROJECT_DIR}/docker/orthanc/plugins/${PLUGIN_TARGET}.so)
