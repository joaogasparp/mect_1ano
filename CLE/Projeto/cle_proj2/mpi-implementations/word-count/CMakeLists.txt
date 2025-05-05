cmake_minimum_required(VERSION 3.22.1)
project(base-mpi CXX)

# Require MPI for this project:
find_package(MPI REQUIRED)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
include_directories(${MPI_INCLUDE_PATH})

# Executável para MPI com o nome cle-wc-mpi
add_executable(cle-wc-mpi  
    src/main.cpp
    src/word_count.cpp
    src/utf-8.cpp
)
target_link_libraries(cle-wc-mpi ${MPI_LIBRARIES})
target_compile_features(cle-wc-mpi PRIVATE cxx_std_20)
target_compile_options(cle-wc-mpi PRIVATE -Wall -Wextra)
