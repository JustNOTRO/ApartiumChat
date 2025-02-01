cmake_minimum_required(VERSION 3.30)
project(ApartiumChat)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/tcp_server)

add_executable(ApartiumChat
    server/server.cpp
    tcp_client/client.cpp
    tcp_client/Client.cpp
    tcp_client/Client.h
    tcp_server/Server.cpp
    tcp_server/Server.h
)
