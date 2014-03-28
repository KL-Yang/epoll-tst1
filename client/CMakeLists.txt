cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(rfs_client)

add_executable(rfs_client main.c)
target_link_libraries (rfs_client uv)
