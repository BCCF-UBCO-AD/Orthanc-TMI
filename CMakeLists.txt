cmake_minimum_required(VERSION 3.20)
project(Orthanc_TMI)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)
