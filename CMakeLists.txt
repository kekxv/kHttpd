cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

add_subdirectory(kHttpd)
add_subdirectory(libCarNumOcr)
set(CMAKE_CXX_STANDARD 14)

include_directories(
        .
        kHttpd/H
        libCarNumOcr
)
if (APPLE)
    link_directories(. /usr/local/lib/)
endif ()

add_executable(kHttpdDemo
        main.cpp
        )
TARGET_LINK_LIBRARIES(kHttpdDemo dl kHttpd ${CarNumOcr_LIBS})

add_executable(kCGITest
        Test/kCGITest.cpp
        )
TARGET_LINK_LIBRARIES(kCGITest dl kHttpd)
add_executable(udpServer
        Test/UdpServer.cpp
        )
TARGET_LINK_LIBRARIES(udpServer dl kHttpd)


find_library(dl dl)
find_library(pthread pthread)
find_library(libevent libevent)

if (APPLE)
    set(LIBEVENT_INCLUDE_DIRS /usr/local/include/)
    link_directories(. /usr/local/lib/)
endif ()
include_directories(.
        ${LIBEVENT_INCLUDE_DIRS}
        )
link_directories(.)
add_executable(test
        Test/test.cpp
        )
TARGET_LINK_LIBRARIES(test dl kHttpd event)