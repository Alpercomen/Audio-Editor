cmake_minimum_required(VERSION 3.20)
project(Sona LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(SetupConfig)
include(HandleQt)

add_subdirectory(External)
add_subdirectory(Source/Application)

target_compile_definitions(Sona PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")