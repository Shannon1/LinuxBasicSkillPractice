cmake_minimum_required(VERSION 3.7)
project(lbsp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp backlog.h)
add_executable(lbsp ${SOURCE_FILES})