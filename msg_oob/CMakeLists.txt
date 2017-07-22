cmake_minimum_required(VERSION 3.7)
project(lbsp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES )
add_executable(msg_oob_send ${SOURCE_FILES} msg_oob_send.cpp)
add_executable(msg_oob_recv ${SOURCE_FILES} msg_oob_recv.cpp)