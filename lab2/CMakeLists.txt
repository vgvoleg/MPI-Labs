cmake_minimum_required(VERSION 3.6)
project(lab2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


# MPI
find_package( MPI REQUIRED QUIET)
include_directories( ${MPI_INCLUDE_PATH} )

set(SOURCE_FILES main.cpp)
add_executable(lab2 ${SOURCE_FILES})

target_link_libraries( ${PROJECT_NAME} ${MPI_LIBRARIES} )