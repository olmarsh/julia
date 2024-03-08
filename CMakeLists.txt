cmake_minimum_required(VERSION 3.27)
project(Julia)
set(CMAKE_CXX_STANDARD 23)

include_directories(lib/lodepng)

add_library(
    lodepng
    lib/lodepng/lodepng.cpp
    lib/lodepng/lodepng.h
)

include(FetchContent)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
    message(STATUS "Fetching yaml-cpp...")
    FetchContent_Populate(yaml-cpp)
    add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

add_executable(Julia Main.cpp)

target_link_libraries(Julia
        PUBLIC lodepng
        PUBLIC yaml-cpp::yaml-cpp
)
