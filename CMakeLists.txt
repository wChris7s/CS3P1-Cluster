cmake_minimum_required(VERSION 3.29)
project(CS3P1_Cluster)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)

add_executable(CS3P1_Cluster
        source/mpi_mat_vect_mult.cpp
        source/hello_world.cpp
        source/mpi_odd_even.cpp
)

target_link_libraries(CS3P1_Cluster PRIVATE MPI::MPI_CXX)