cmake_minimum_required(VERSION 3.22.1)
project(base-mpi CXX)

# Require MPI for this project:
find_package(MPI REQUIRED)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
include_directories(${MPI_INCLUDE_PATH})

add_executable(hello src/hello.cpp)
target_link_libraries(hello ${MPI_LIBRARIES})
target_compile_features(hello PRIVATE cxx_std_20)
target_compile_options(hello PRIVATE -Wall -Wextra)

add_executable(error src/error.cpp)
target_link_libraries(error ${MPI_LIBRARIES})
target_compile_features(error PRIVATE cxx_std_20)
target_compile_options(error PRIVATE -Wall -Wextra)

add_executable(error-return src/error-return.cpp)
target_link_libraries(error-return ${MPI_LIBRARIES})
target_compile_features(error-return PRIVATE cxx_std_20)
target_compile_options(error-return PRIVATE -Wall -Wextra)

add_executable(sendRecData src/sendRecData.cpp)
target_link_libraries(sendRecData ${MPI_LIBRARIES})
target_compile_features(sendRecData PRIVATE cxx_std_20)
target_compile_options(sendRecData PRIVATE -Wall -Wextra)
