cmake_minimum_required(VERSION 3.12)
project(GM_Server C)

set(CMAKE_C_STANDARD 99)

add_executable(GM_Server gm_server.c authorize.h authorize.c)
