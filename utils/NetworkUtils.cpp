#include <iostream>
#include "NetworkUtils.h"
#include "ServerConstants.h"
#include "Logger.h"
#include "SocketAdapter.h"

std::string NetworkUtils::getSelectedIpAddress(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    return ipAddress.substr(0, colonPos);
}

std::uint16_t NetworkUtils::getSelectedPort(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    if (colonPos == std::string::npos || colonPos + 1 == ipAddress.length()) {
        return DEFAULT_PORT;
    }

    // Parse the port as short since we don't need that many bytes.
    return std::stoi(ipAddress.substr(colonPos + 1));
}

Socket NetworkUtils::createSocket() {
    #ifdef _WIN32
        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            Logger::logLastError("WSAStartup failed");
            exit(EXIT_FAILURE);
        }
    #endif // _WIN32

    Socket newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (newSock < 0) {
        Logger::logLastError("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    return newSock;
}

void NetworkUtils::closeSocket(Socket sock) {
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif // _WIN32
}
