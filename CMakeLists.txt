cmake_minimum_required(VERSION 3.5)

project(PjPlots)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cpp)

if (PJPLOTS_ENABLE_TESTS MATCHES ON)
    message("Testing enabled")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PJPLOT_ENABLE_TESTS)
endif()