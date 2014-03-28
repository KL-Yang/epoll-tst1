cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(rfs_server)

add_executable(rfs_server main.c)
target_link_libraries (rfs_server uv)
