cmake_minimum_required(VERSION 3.1)
project(a2 LANGUAGES CXX)

set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

set(CMAKE_CXX_STANDARD 14)

add_executable(a2 
  ${SOURCE_DIR}/main.cpp
  ${SOURCE_DIR}/parser.h
  ${SOURCE_DIR}/parser.cpp
  ${SOURCE_DIR}/assembler.h
  ${SOURCE_DIR}/assembler.cpp
)

message("------------------------------------")
message("'${CMAKE_GENERATOR}' is used to build this project")
message("source files: ${SOURCE_DIR}")
message("------------------------------------")
