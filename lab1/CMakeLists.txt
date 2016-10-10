cmake_minimum_required(VERSION 3.6)
project(lab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")


# MPI
find_package( MPI REQUIRED QUIET)
include_directories( ${MPI_INCLUDE_PATH} )

set(SOURCE_FILES main.c)
add_executable(lab1 ${SOURCE_FILES})

target_link_libraries( ${PROJECT_NAME} ${MPI_LIBRARIES} )