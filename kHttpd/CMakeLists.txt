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

if(LIBEVENT_LIBRARIES MATCHES "")
    set(LIBEVENT_LIBRARIES event event_core event_extra)
endif()

include_directories(.
        H
        ${LIBEVENT_INCLUDE_DIRS}
        )
link_directories(.)

aux_source_directory(Cpp CppSource)


add_library(kHttpd STATIC
            ${CppSource}
        )
TARGET_LINK_LIBRARIES(kHttpd dl ${LIBEVENT_LIBRARIES})