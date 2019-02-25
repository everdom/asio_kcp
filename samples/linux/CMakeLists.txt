cmake_minimum_required (VERSION 2.8.9)

project (kcp-sample)


set(KCP_CLIENT_DIR "/home/lwq/source/asio-kcp")
set(KCP_CLIENT_HEAD ${KCP_CLIENT_DIR}/include)
set(KCP_CLIENT_SRC ${KCP_CLIENT_DIR}/src)
set(KCP_CLIENT_LIB ${KCP_CLIENT_DIR}/sdk/linux/${CMAKE_BUILD_TYPE})

set (ESSENTIAL_DIR ../../essential)
set (UTIL_DIR ../../util)
set (CLIENT_DIR ../../client_lib)
include_directories ( ${ESSENTIAL_DIR} )
include_directories ( ${UTIL_DIR} )
include_directories ( ${CLIENT_DIR} )


set(SRC_LIST 
	main.cpp
	)

set(INCLUDE_LIST ${KCP_CLIENT_HEAD})

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -frtti -fexceptions -Wall -pedantic" )  # -fdump-class-hierarchy : dump RTTI etc.
endif ()

link_directories(${KCP_CLIENT_LIB})

include_directories(${INCLUDE_LIST})

add_executable(kcp-sample ${SRC_LIST})

target_link_libraries(kcp-sample kcp_client_static)
