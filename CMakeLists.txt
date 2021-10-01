cmake_minimum_required(VERSION 3.20)
project(Orthanc_TMI)

set(CMAKE_CXX_STANDARD 14)

add_library(Orthanc_TMI SHARED library.cpp)
