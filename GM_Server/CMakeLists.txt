cmake_minimum_required(VERSION 3.12)
project(GM_Server C)

set(CMAKE_C_STANDARD 99)

add_executable(GM_Server gm_server.c server_auth.h server_auth.c server_sockets.c server_sockets.h core.c core.h server_encryption.c server_encryption.h)
