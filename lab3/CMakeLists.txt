cmake_minimum_required(VERSION 2.8)
project(lab3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# MPI
find_package( MPI REQUIRED)
include_directories( ${MPI_INCLUDE_PATH} )

set(SOURCE_FILES main.cpp)
add_executable(lab3 ${SOURCE_FILES})

target_link_libraries( ${PROJECT_NAME} ${MPI_LIBRARIES} )