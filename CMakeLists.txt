cmake_minimum_required(VERSION 3.10)

project(JITInterpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(jit jit.cpp)
