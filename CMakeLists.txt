cmake_minimum_required(VERSION 3.21.0) # needed for C17
project(craftinginterpreters C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
add_subdirectory(clox)
add_subdirectory(jlox-in-cpp)

set(CMAKE_CTEST_ARGUMENTS --output-on-failure)
include(CTest)
add_subdirectory(test)
