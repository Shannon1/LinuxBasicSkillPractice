cmake_minimum_required(VERSION 3.7)
project(lbsp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES )
add_executable(tcp_recv_buf ${SOURCE_FILES} tcp_recv_buf.cpp)
add_executable(tcp_send_buf ${SOURCE_FILES} tcp_send_buf.cpp)