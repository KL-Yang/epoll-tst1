cmake_minimum_required (VERSION 2.6)

project (rfs)
#setup my own find module from internet!
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Glib REQUIRED)

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(common)

