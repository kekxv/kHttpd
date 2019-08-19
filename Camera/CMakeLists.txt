cmake_minimum_required(VERSION 2.8)
project(Camera)
set(CMAKE_CXX_STANDARD 14)

set(Camera_LIBS Camera CACHE INTERNAL "")

find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS})

add_library(Camera
        Camera.cpp
        )
target_link_libraries(Camera ${OpenCV_LIBS})


