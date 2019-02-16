cmake_minimum_required (VERSION 2.8.9)
set (ESSENTIAL_DIR ../essential)
set (UTIL_DIR ../util)
include_directories ( ${ESSENTIAL_DIR} )
include_directories ( ${UTIL_DIR} )

set(CLIENT_SRC_FILES
    kcp_buffer_data.cpp
    kcp_client.cpp
    kcp_client_util.cpp
    kcp_client_wrap.cpp
)

set(ESSENTIAL_SRC_FILES
    ${ESSENTIAL_DIR}/_src/check_function.cpp
    ${ESSENTIAL_DIR}/_src/utility/strutil.cpp
)

set(UTIL_SRC_FILES
    ${UTIL_DIR}/connect_packet.cpp
    ${UTIL_DIR}/ikcp.c
)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -frtti -fexceptions -fPIC" )  # -fdump-class-hierarchy : dump RTTI etc.
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")
endif ()

IF (WIN32)
    set(LIBS_SYSTEM ws2_32)
ELSEIF (UNIX)
    IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LIBS_SYSTEM c dl pthread)
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(LIBS_SYSTEM compat pthread)
    ELSE()
        set(LIBS_SYSTEM c pthread)
    ENDIF()
ENDIF()

add_library(kcp_client_static 
    STATIC 
    ${CLIENT_SRC_FILES}
    ${ESSENTIAL_SRC_FILES}
    ${UTIL_SRC_FILES}
    )

add_library(kcp_client 
    SHARED 
    ${CLIENT_SRC_FILES}
    ${UTIL_SRC_FILES}
    )

target_link_libraries(kcp_client ${LIBS_SYSTEM})