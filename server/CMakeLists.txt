cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(rfs_server)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Glib REQUIRED)
find_package(Libsoup REQUIRED)

include_directories(${GLIB_INCLUDE_DIRS} ${LIBSOUP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLIB_LIBRARIES} ${LIBSOUP_LIBRARIES})

add_executable(rfs_server main.c bind.c accept.c inbound.c dispatch.c
    outbound.c open.c close.c read.c write.c
    ../common/socket_connect.c context.c)
target_link_libraries (rfs_server ${LIBS} uv)
