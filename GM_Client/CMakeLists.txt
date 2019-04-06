cmake_minimum_required(VERSION 3.12)
project(GM_Client C)

set(CMAKE_C_STANDARD 99)

add_executable(GM_Client gm_client.c client_sockets.c client_sockets.h client_auth.c client_auth.h core.c core.h)