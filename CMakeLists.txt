cmake_minimum_required(VERSION 3.16)
project(Histogram)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")

add_executable(Histogram main.cpp)
target_link_libraries(Histogram pthread)
