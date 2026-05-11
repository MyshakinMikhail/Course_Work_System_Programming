cmake_minimum_required(VERSION 3.16)

project(Course_Work_System_Programming LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(src/storage)
add_subdirectory(src/core)
add_subdirectory(tests)
