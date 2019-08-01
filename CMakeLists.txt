cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

add_subdirectory(kHttpd)
set(CMAKE_CXX_STANDARD 14)

include_directories(
        .
        kHttpd/H
)
if (APPLE)
    link_directories(. /usr/local/lib/)
endif ()

add_executable(kHttpdDemo
        main.cpp
        )
TARGET_LINK_LIBRARIES(kHttpdDemo dl kHttpd)

add_executable(kCGITest
        Test/kCGITest.cpp
        )
TARGET_LINK_LIBRARIES(kCGITest dl kHttpd)