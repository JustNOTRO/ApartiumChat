cmake_minimum_required(VERSION 3.10)
project(TCPProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/tcp_server ${CMAKE_SOURCE_DIR}/tcp_client)

add_executable(server
    tcp_server/main.cpp
    tcp_server/ThreadPool.cpp
    tcp_server/Server.cpp
    tcp_client/Client.cpp
    tcp_server/utils/ServerUtils.cpp
)

add_executable(client
    tcp_client/main.cpp
    tcp_client/Client.cpp
    tcp_server/Server.cpp
    tcp_server/ThreadPool.cpp
    tcp_server/utils/ServerUtils.cpp
)
