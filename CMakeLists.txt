cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

add_subdirectory(kHttpd)
add_subdirectory(libCarNumOcr)

option(ENABLE_CAMERA "option for CAMERA" ON)
if (ENABLE_CAMERA)
    add_definitions(-DENABLE_CAMERA)
    add_subdirectory(Camera)
    find_package(OpenCV REQUIRED)
endif ()

set(CMAKE_CXX_STANDARD 14)

include_directories(
        .
        kHttpd/H
        libCarNumOcr
)

find_library(dl dl)
find_library(pthread pthread)
find_library(Libevent Libevent)
include_directories(.
        Camera
        ${LIBEVENT_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        )

if (APPLE)
    link_directories(. /usr/local/lib/)
    set(LIBEVENT_INCLUDE_DIRS /usr/local/include/)
    set(LIBEVENT_LIBRARIES event)
    link_directories(. /usr/local/lib/)
endif ()
if (LIBEVENT_LIBRARIES MATCHES "")
    set(LIBEVENT_LIBRARIES event event_core event_extra)
endif ()

add_executable(kHttpdDemo
        main.cpp
        )
TARGET_LINK_LIBRARIES(kHttpdDemo dl kHttpd ${CarNumOcr_LIBS} ${LIBEVENT_LIBRARIES})

add_executable(kCGITest
        Test/kCGITest.cpp
        )
TARGET_LINK_LIBRARIES(kCGITest dl kHttpd)
add_executable(mysqlTest
        Test/mysqlTest.cpp
        )
TARGET_LINK_LIBRARIES(mysqlTest dl pthread kHttpd)
add_executable(udpServer
        Test/UdpServer.cpp
        )
TARGET_LINK_LIBRARIES(udpServer kHttpd ${OpenCV_LIBS} ${Camera_LIBS} ${CarNumOcr_LIBS} ${LIBEVENT_LIBRARIES} dl pthread)


link_directories(.)
add_executable(test_tcp
        Test/test.cpp
        )
TARGET_LINK_LIBRARIES(test_tcp dl kHttpd ${LIBEVENT_LIBRARIES})