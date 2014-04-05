cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(rfs_client)

#setup my own find module from internet!
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Glib REQUIRED)
find_package(Libsoup REQUIRED)

include_directories(${GLIB_INCLUDE_DIRS} ${LIBSOUP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLIB_LIBRARIES} ${LIBSOUP_LIBRARIES})

add_executable(rfs_client main.c open.c close.c write.c socket_connect.c)
target_link_libraries (rfs_client ${LIBS})
