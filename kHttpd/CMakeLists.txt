cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst $(dir $<),,$<)\"'")
set(CMAKE_CXX_STANDARD 14)
add_definitions(
        -DEN_REGEX
)
find_library(dl dl)
find_library(pthread pthread)
find_library(Libevent Libevent)

if (APPLE)
    set(LIBEVENT_INCLUDE_DIRS /usr/local/include/)
    set(LIBEVENT_LIBRARIES event)
    link_directories(. /usr/local/lib/)
endif ()

if (LIBEVENT_LIBRARIES MATCHES "")
    set(LIBEVENT_LIBRARIES event event_core event_extra)
endif ()
include_directories(.
        H
        ${LIBEVENT_INCLUDE_DIRS}
        )
link_directories(.)

aux_source_directory(Cpp CppSource)

add_library(kHttpd STATIC
        ${CppSource}
        )

include(ExternalProject)
option(BUILD_MYSQL "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_MYSQL)
    ExternalProject_Add(mysqlclient
            URL https://downloads.mysql.com/archives/get/file/mysql-connector-c-6.1.11-src.tar.gz
            URL_MD5 "98ca2071f9d4c6b73146cc0455f6b914"
            CONFIGURE_COMMAND cmake -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
            PREFIX ${CMAKE_BINARY_DIR}/dependencies
            INSTALL_DIR ${INSTALL_DIR}
            BUILD_COMMAND ${MAKE}
            )
    set(mysqlclient_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
    if (WIN32)
        set(prefix "")
        set(suffix ".lib")
    else ()
        set(prefix "lib")
        set(suffix ".a")
    endif ()
    set(mysqlclient_LIBRARIES
            "${mysqlclient_LIB_DIR}/${prefix}mysqlclient${suffix}"
            "${mysqlclient_LIB_DIR}/${prefix}mysqlclient${suffix}")

    include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
    add_dependencies(kHttpd mysqlclient)
endif ()

option(BUILD_SQLITE3 "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_MYSQL)
    ExternalProject_Add(sqlite3
            URL https://www.sqlite.org/2019/sqlite-autoconf-3290000.tar.gz
            URL_MD5 "8f3dfe83387e62ecb91c7c5c09c688dc"
            CONFIGURE_COMMAND <SOURCE_DIR>/configure
            PREFIX ${CMAKE_BINARY_DIR}/dependencies
            INSTALL_DIR ${INSTALL_DIR}
            BUILD_COMMAND ${MAKE}
            )
    set(sqlite3_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
    if (WIN32)
        set(prefix "")
        set(suffix ".lib")
    else ()
        set(prefix "lib")
        set(suffix ".a")
    endif ()
    set(sqlite3_LIBRARIES
            "${sqlite3_LIB_DIR}/${prefix}sqlite3${suffix}"
            "${sqlite3_LIB_DIR}/${prefix}sqlite3${suffix}")

    include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
    add_dependencies(kHttpd sqlite3)
endif ()

TARGET_LINK_LIBRARIES(kHttpd ${LIBEVENT_LIBRARIES} ${sqlite3_LIBRARIES} ${mysqlclient_LIBRARIES} dl pthread)