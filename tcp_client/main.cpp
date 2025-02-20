//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include "utils/ServerUtils.h"

bool sendHeartbeat(const int &sock) {
    if (send(sock, ServerUtils::HEARTBEAT_REQUEST, sizeof(ServerUtils::HEARTBEAT_MESSAGE), 0) < 0) {
        std::cerr << "Could not send heartbeat: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Sent ping to server." << std::endl;
    return true;
}

bool isServerResponding(const int &sock, int &readySocks, char buffer[]) {
    int bytesReceived = recv(sock, buffer, ServerUtils::BUFFER_SIZE, 0);
    if (bytesReceived < 0) {
        std::cerr << "Error while trying to receive data from server: " << strerror(errno) << std::endl;
        return false;
    }

    if (bytesReceived == 0) {
        std::cout << "Server is not responding.. disconnecting.." << std::endl;
        return false;
    }

    return true;
}

void handleIncomingMessages(const int &sock) {
    char buffer[ServerUtils::BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, ServerUtils::BUFFER_SIZE);

        struct timeval timeout;
        timeout.tv_sec = ServerUtils::HEARTBEAT_INTERVAL;
        timeout.tv_usec = 0;

        fd_set readSocks;
        fd_set *readSocksAddr = &readSocks;

        FD_ZERO(readSocksAddr);
        FD_SET(sock, readSocksAddr);

        int readySocks = select(sock + 1, readSocksAddr, nullptr, nullptr, &timeout);
        if (readySocks < 0) {
            std::cerr << "Error during select(): " << strerror(errno) <<  ". Disconnecting..." << std::endl;
            break;
        }

        sendHeartbeat(sock);

        if (!isServerResponding(sock, readySocks, buffer)) {
            break;
        }

        if (strcmp(ServerUtils::HEARTBEAT_RESPONSE, buffer) != 0) {
            std::cout << buffer << std::endl; // print the message if it's not heartbeat response
        }

    }

    close(sock);
}

bool handleClientInput(const int &sock, std::string &msg) {
   if (msg.length() > ServerUtils::BUFFER_SIZE) {
       std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
       return false;
   }

   if (msg == "/exit") {
       return false;
   }

   if (msg == "") {
      return false;
   }

   if (msg == "/ping") {
      return false;
   }

   return true;
}

bool connectToServer(std::string& ipAddress, const int& sock) {
    const short& port = ServerUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    const std::string& ip = ServerUtils::getSelectedIpAddress(ipAddress);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        return false;
    }

    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

int createSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main() {
    const int& sock = createSocket();

    std::cout << "Enter IP Address: ";
    std::string ipAddress;
    std::cin >> ipAddress;

    if (!connectToServer(ipAddress, sock)) {
        std::cerr << "Could not connect to server: " << ipAddress << std::endl;
        return 1;
    }

    std::cin.clear();

    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;

    send(sock, username.c_str(), username.length(), 0); // Sending username

    std::thread(handleIncomingMessages, sock).detach();

    char buffer[ServerUtils::BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, ServerUtils::BUFFER_SIZE);
        std::cin.getline(buffer, ServerUtils::BUFFER_SIZE);
        std::string msg = std::string(buffer);

        if (!handleClientInput(sock, msg)) {
            break;
        }

        if (send(sock, buffer, ServerUtils::BUFFER_SIZE, 0) == -1) {
            std::cerr << "Could not send message to server: " << ipAddress << std::endl;
            return 1;
        }
    }

    close(sock);
    return 0;
}