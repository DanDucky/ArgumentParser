cmake_minimum_required(VERSION 3.24)
project(Examples)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(Examples example_0.cpp)

target_include_directories(Examples PUBLIC ${CMAKE_SOURCE_DIR}/../include)
target_link_libraries(Examples stdc++)
