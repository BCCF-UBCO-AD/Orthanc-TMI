cmake_minimum_required(VERSION 3.20)
project(Orthanc_TMI)

set(CMAKE_CXX_STANDARD 14)


include_directories(src)

add_library(Orthanc_TMI SHARED library.cpp)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)