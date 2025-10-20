cmake_minimum_required(VERSION 3.20)
project(cache
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SANITIZE "Enable compiler sanitizers" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

add_subdirectory(source)
if (BUILD_TESTS)
        add_subdirectory(tests)
endif()