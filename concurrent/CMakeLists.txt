project(lock_free)
cmake_minimum_required(VERSION 3.6)

INCLUDE_DIRECTORIES(
    ./
    ../../3rd/google-benchmark/include
)

LINK_DIRECTORIES(
    /usr/local/lib
    /usr/lib
    /usr/lib64
    ../../3rd/google-benchmark/lib
)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++")
#set(CMAKE_BUILD_TYPE Debug)
# we can create Makefile for debug or release: cmake -DCMAKE_BUILD_TYPE=Release ${project-path}
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -Wall -pg -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")


aux_source_directory(./ SRC_LIST)
add_library(lock_free ${SRC_LIST})
set_target_properties(lock_free PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(lock_free)

add_executable(low_lock_queue_benchmark low_lock_queue_benchmark.cpp)
target_link_libraries(low_lock_queue_benchmark lock_free libbenchmark.a rt pthread)

add_executable(low_lock_queue_test low_lock_queue_test.cpp)
target_link_libraries(low_lock_queue_test lock_free pthread)
