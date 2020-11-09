cmake_minimum_required(VERSION 3.0)
project(Histogram)

find_package(MPI REQUIRED)

set(CMAKE_CXX_STANDARD 11)

include_directories(${MPI_CXX_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3"  ${MPI_CXX_COMPILE_FLAGS})

add_executable(Histogram main.cpp)
target_link_libraries(Histogram ${MPI_CXX_LIBRARIES})
