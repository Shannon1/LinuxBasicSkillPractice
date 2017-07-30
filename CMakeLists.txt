cmake_minimum_required(VERSION 3.7)
project(lbsp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES )
#add_executable(lbsp ${SOURCE_FILES})

add_subdirectory(msg_oob)
add_subdirectory(tcp_buf)
add_subdirectory(accept)
add_subdirectory(backlog)
add_subdirectory(daytime_info)
add_subdirectory(cgi)
add_subdirectory(writev)
add_subdirectory(sendfile)
add_subdirectory(splice)
add_subdirectory(tee)
add_subdirectory(http)
