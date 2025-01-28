cmake_minimum_required(VERSION 3.30)
project(ApartiumChat)

set(CMAKE_CXX_STANDARD 20)

add_executable(ApartiumChat server/server.cpp
        client/client.cpp
        client/Client.cpp
        client/Client.h
        server/Server.cpp
        server/Server.h
)
