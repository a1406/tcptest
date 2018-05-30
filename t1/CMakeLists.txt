CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(XGame)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake/")

option(MEMERR
    "check mem overflow" ON)

add_subdirectory(comm_lib)
add_subdirectory(conn_srv)
add_subdirectory(game_srv)
