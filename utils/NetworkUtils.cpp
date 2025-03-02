#include <iostream>
#include <cstring>
#include <memory>
#include "NetworkUtils.h"
#include "ServerConstants.h"

std::string NetworkUtils::getSelectedIpAddress(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    if (colonPos == std::string::npos) {
        colonPos = ipAddress.size();
    }

    return ipAddress.substr(0, colonPos);
}

std::uint16_t NetworkUtils::getSelectedPort(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    size_t lastIndex = ipAddress.length() - 1;
    if (colonPos == std::string::npos || colonPos == lastIndex) {
        return DEFAULT_PORT;
    }

    // Parse the port as std::uint16_t since we don't need that many bytes.
    return std::stoi(ipAddress.substr(colonPos + 1));
}

#ifdef _WIN32
std::string NetworkUtils::getLastWindowsError() {
    int errorCode = WSAGetLastError();
    char *msgBuffer = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuffer, 0, nullptr
    );

    std::unique_ptr<char, decltype(&LocalFree)> msg(msgBuffer, LocalFree);
    if (!msg) {
        std::cerr << "Could not retrieve windows error message." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return std::string(msg.get());
}
#endif // _WIN32

std::string NetworkUtils::getLastError() {
    #ifdef _WIN32
        return getLastWindowsError();
    #else
        return strerror(errno);
    #endif // _WIN32
}

Socket NetworkUtils::createSocket() {
    #ifdef _WIN32
        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            std::cerr << "WSAStartup failed: " << getLastWindowsError() << std::endl; 
            exit(EXIT_FAILURE);
        }
    #endif // _WIN32

    Socket newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (newSock < 0) {
        std::cerr << "Socket creation failed: " << getLastError() << "." << std::endl;
        exit(EXIT_FAILURE);
    }

    return newSock;
}

void NetworkUtils::closeSocket(Socket sock) {
    #ifdef _WIN32
        closesocket(sock);
    #else
        close(sock);
    #endif // _WIN32
}

void NetworkUtils::shutdownSocket(Socket sock) {
    #ifdef _WIN32
        shutdown(sock, SD_BOTH);
    #else
        shutdown(sock, SHUT_RDWR);
    #endif
}
