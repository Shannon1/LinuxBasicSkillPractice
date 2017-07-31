cmake_minimum_required(VERSION 3.7)
project(lbsp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES )
add_executable(ltet ${SOURCE_FILES} ltet.cpp)
add_executable(oneshot ${SOURCE_FILES} oneshot.cpp)
target_link_libraries(oneshot pthread)
