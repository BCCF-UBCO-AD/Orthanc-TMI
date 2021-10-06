cmake_minimum_required(VERSION 3.20)
project(Orthanc_TMI)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
set(PLUGIN_NAME DataAnonymizer)
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})


include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(lib/googletest)
