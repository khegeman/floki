PROJECT(floki)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)



include_directories(.)
 

ADD_DEFINITIONS(
  -std=c++11
   -Wall
)

add_subdirectory(test)

enable_testing()


add_executable(sort bench/sort.cpp)

add_executable(sort_simd bench/sort.cpp)
set_target_properties(sort_simd PROPERTIES COMPILE_DEFINITIONS SIMD_BENCH)
