cmake_minimum_required(VERSION 3.10)
project(ApartiumChat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/tcp_server
    ${CMAKE_SOURCE_DIR}/tcp_client
    ${CMAKE_SOURCE_DIR}/utils
)

if(WIN32)
    if(MSVC)
        set(CMAKE_C_COMPILER "cl.exe")
        set(CMAKE_CXX_COMPILER "cl.exe")
    elseif(MINGW)
        set(CMAKE_C_COMPILER "C:/mingw-w64/bin/x86_64-w64-mingw32-gcc")
        set(CMAKE_CXX_COMPILER "C:/mingw-w64/bin/x86_64-w64-mingw32-g++")
    
    set(CMAKE_FIND_ROOT_PATH "C:/mingw-w64")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

elseif(APPLE)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    
elseif(UNIX)
    # For Linux, use GCC or Clang as the default compilers
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_C_COMPILER "gcc")
endif()

add_executable(server
    tcp_server/main.cpp
    tcp_server/ThreadPool.cpp
    tcp_server/Server.cpp
    tcp_client/Client.cpp
    utils/NetworkUtils.cpp
    utils/Logger.cpp
)

add_executable(client
    tcp_client/main.cpp
    tcp_client/Client.cpp
    tcp_server/Server.cpp
    tcp_server/ThreadPool.cpp
    utils/NetworkUtils.cpp
    utils/Logger.cpp
)

if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)

elseif(APPLE)
    target_link_libraries(server pthread)
    target_link_libraries(client pthread)

elseif(UNIX)
    target_link_libraries(server pthread)
    target_link_libraries(client pthread)
endif()
