cmake_minimum_required(VERSION 3.10)
project(msgexample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include msgs directory for headers
include_directories(${CMAKE_SOURCE_DIR}/../msgs)
add_executable(msgexample msgexample.cc)

# If you need to include the msgs headers
target_include_directories(msgexample PRIVATE ${CMAKE_SOURCE_DIR}/msgs)
