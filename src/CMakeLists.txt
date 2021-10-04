file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.cpp)
add_library(DataAnonymizer SHARED ${SOURCES})
