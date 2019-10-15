cmake_minimum_required(VERSION 2.8)
project(kHttpd)
find_library(PkgConfig REQUIRED)

if (APPLE)
    set(OpenCV_DIR /usr/local/lib/cmake/opencv4/)
    set(Libevent_DIR /usr/local/lib/cmake/libevent/)
endif()
add_subdirectory(kHttpd)
add_subdirectory(libCarNumOcr)

option(ENABLE_CAMERA "option for CAMERA" ON)
if (ENABLE_CAMERA)
    add_definitions(-DENABLE_CAMERA)
    add_subdirectory(Camera)
endif ()

set(CMAKE_CXX_STANDARD 14)

include_directories(
        .
        kHttpd/H
        libCarNumOcr
)

find_package(OpenCV REQUIRED)
find_library(dl dl)
find_library(pthread pthread)
find_package(Libevent REQUIRED)

include_directories(.
        Camera
        ${kHttpd_INCLUDE_DIRS}
        ${LIBEVENT_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        )

if (APPLE)
    link_directories(/usr/local/lib/)
endif ()
if (LIBEVENT_LIBRARIES MATCHES "")
    set(LIBEVENT_LIBRARIES event)
endif ()

add_executable(kHttpdDemo
        main.cpp
        )
TARGET_LINK_LIBRARIES(kHttpdDemo dl kHttpd ${CarNumOcr_LIBS} ${LIBEVENT_LIBRARIES})

add_executable(kCGITest
        Test/kCGITest.cpp
        )
TARGET_LINK_LIBRARIES(kCGITest dl kHttpd)
option(BUILD_MYSQL "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_MYSQL)
add_executable(mysqlTest
        Test/mysqlTest.cpp
        )
TARGET_LINK_LIBRARIES(mysqlTest dl pthread kHttpd)

INSTALL(TARGETS mysqlTest
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION libstatic
        )
endif()

option(BUILD_SQLITE3 "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_SQLITE3)
add_executable(sqliteTest
        Test/sqliteTest.cpp
        )
TARGET_LINK_LIBRARIES(sqliteTest dl pthread kHttpd)

INSTALL(TARGETS sqliteTest
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION libstatic
        )
endif()

add_executable(udpServer
        Test/UdpServer.cpp
        )
TARGET_LINK_LIBRARIES(udpServer kHttpd ${OpenCV_LIBS} ${Camera_LIBS} ${CarNumOcr_LIBS} ${LIBEVENT_LIBRARIES} dl pthread)


#link_directories(.)
add_executable(test_tcp
        Test/test.cpp
        )
TARGET_LINK_LIBRARIES(test_tcp dl kHttpd ${LIBEVENT_LIBRARIES})

INSTALL(TARGETS test_tcp udpServer kCGITest kHttpdDemo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION libstatic
        )