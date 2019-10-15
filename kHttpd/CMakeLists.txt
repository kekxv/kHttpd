cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst $(dir $<),,$<)\"'")
set(CMAKE_CXX_STANDARD 14)
add_definitions(
        -DEN_REGEX
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
    set(LIBEVENT_LIBRARIES event)
    link_directories(/usr/local/lib/)
endif ()

if (LIBEVENT_LIBRARIES MATCHES "")
    set(LIBEVENT_LIBRARIES event)
endif ()

set(kHttpd_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/kHttpd/CGI
        ${CMAKE_SOURCE_DIR}/kHttpd/database
        ${CMAKE_SOURCE_DIR}/kHttpd/ObjectDetection
        ${CMAKE_SOURCE_DIR}/kHttpd/http
        ${CMAKE_SOURCE_DIR}/kHttpd/json
        ${CMAKE_SOURCE_DIR}/kHttpd/Tools
        ${CMAKE_SOURCE_DIR}/kHttpd/udp
        CACHE INTERNAL ""
        )

include_directories(.
        ${kHttpd_INCLUDE_DIRS}
        ${LIBEVENT_INCLUDE_DIRS}
        )
#link_directories(.)

aux_source_directory(CGI CGISource)
aux_source_directory(database databaseSource)
aux_source_directory(ObjectDetection ObjectDetectionSource)
aux_source_directory(http httpSource)
aux_source_directory(json jsonSource)
aux_source_directory(Tools ToolsSource)
aux_source_directory(udp udpSource)

add_library(kHttpd STATIC
        ${CGISource}
        ${databaseSource}
        ${ObjectDetectionSource}
        ${httpSource}
        ${jsonSource}
        ${ToolsSource}
        ${udpSource}
        )

include(ExternalProject)
option(BUILD_MYSQL "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_MYSQL)
    add_definitions(-DBUILD_MYSQL)
    ExternalProject_Add(mysqlclient
            URL https://github.com/kekxv/mysqlclient/archive/master.zip
            URL_MD5 "c4349d24b9bfe3a234d8ef225d021528"
            CONFIGURE_COMMAND cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
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
            "${mysqlclient_LIB_DIR}/${prefix}mysqlclient${suffix}")

    include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
    add_dependencies(kHttpd mysqlclient)
endif ()

option(BUILD_SQLITE3 "Build the CppUnit library (optional; used for unit testing)" ON)
if (BUILD_SQLITE3)
    add_definitions(-DBUILD_SQLITE3)
    ExternalProject_Add(sqlite3
            URL https://www.sqlite.org/2019/sqlite-autoconf-3290000.tar.gz
            URL_MD5 "8f3dfe83387e62ecb91c7c5c09c688dc"
            CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} <SOURCE_DIR>/configure --host=${CMAKE_SYSTEM_PROCESSOR} --prefix=${CMAKE_BINARY_DIR}/dependencies
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

TARGET_LINK_LIBRARIES(kHttpd ${OpenCV_LIBS} ${LIBEVENT_LIBRARIES} ${sqlite3_LIBRARIES} ${mysqlclient_LIBRARIES} dl pthread)
INSTALL(TARGETS kHttpd
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION libstatic
        )