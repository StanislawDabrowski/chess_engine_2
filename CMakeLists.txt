cmake_minimum_required(VERSION 3.23)

project(chess_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(chess_engine_Cpp_2)
add_subdirectory(uci)
add_subdirectory(tests)
add_subdirectory(utils)
