#pragma once

#ifndef SOCKETADAPTER_H
#define SOCKETADAPTER_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

#ifdef _WIN32
    using Socket = SOCKET;
#else
    using Socket = int;
#endif // _WIN32

#endif // SOCKETADAPTER_H