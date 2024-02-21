cmake_minimum_required(VERSION 3.15)
project(julia VERSION 0.1)

include_directories(lib/lodepng)
add_library(
        lodepng
        lib/lodepng/lodepng.cpp
        lib/lodepng/lodepng.h
)

add_executable(main main.cpp)

target_link_libraries(main lodepng)