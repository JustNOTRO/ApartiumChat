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

#define DEFAULT_PORT 3333
#define BUFFER_SIZE 1500
#define HEARTBEAT_INTERVAL 5

#define HEARTBEAT_MESSAGE "/ping"
#define HEARTBEAT_RESPONSE "/pong"

void resetTimeout(const int &sock, timeval &timeout, fd_set *readFds, int &readyFds) {
    timeout.tv_sec = HEARTBEAT_INTERVAL;
    timeout.tv_usec = 0;

    readyFds = select(sock + 1, readFds, nullptr, nullptr, &timeout);
    std::cout << "Assigned readyFds again" << std::endl;
}

bool sendHeartbeat(const int &sock, timeval &timeout, fd_set *set, int &readyFds) {
    if (send(sock, HEARTBEAT_MESSAGE, sizeof(HEARTBEAT_MESSAGE), 0) < 0) {
        std::cerr << "Could not send heartbeat: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Sent ping to server." << std::endl;
    resetTimeout(sock, timeout, set, readyFds);
    return true;
}

bool isServerResponding(const int &sock, int &readyFds, char buffer[]) {
    if (readyFds == 0) {
        std::cerr << "Server is not responding.. Disconnecting.." << std::endl;
        return false;
    }

    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived < 0) {
        std::cerr << "Error while trying to receive data from server: " << strerror(errno) << std::endl;
        return false;
    }

    if (bytesReceived == 0) {
        std::cout << "Server has been closed." << std::endl;
        return false;
    }

    return true;
}



void handleIncomingMessages(const int& sock) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        struct timeval timeout;
        timeout.tv_sec = HEARTBEAT_INTERVAL;
        timeout.tv_usec = 0;

        fd_set readFds;
        fd_set *readFdsAddr = &readFds;

        FD_ZERO(readFdsAddr);
        FD_SET(sock, readFdsAddr);

        int readyFds = select(sock + 1, readFdsAddr, nullptr, nullptr, &timeout);
        if (readyFds < 0) {
            std::cerr << "Error during select(): " << strerror(errno) <<  ". Disconnecting..." << std::endl;
            break;
        }

        if (readyFds == 0) {
            sendHeartbeat(sock, timeout, readFdsAddr, readyFds);
        }

        if (!isServerResponding(sock, readyFds, buffer)) {
            break;
        }

        if (strcmp(HEARTBEAT_RESPONSE, buffer) != 0) {
            std::cout << buffer << std::endl; // print the message if it's not heartbeat response
        }

    }

    // close(sock);
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

// void connectToFallbackServer() {
//     int newSock = socket(AF_INET, SOCK_STREAM, 0);
//     if (newSock < 0) {
//         std::cerr << "Socket creation failed." << std::endl;
//         exit(EXIT_FAILURE);
//     }

//     std::string fallbackAddr = serverAddresses.top();
//     while (!connectToServer(fallbackAddr, newSock)) {
//         serverAddresses.dequeue();
//         fallbackAddr = serverAddresses.top();
//     }

//     std::cout << "Connected to fallback server: " << fallbackAddr << std::endl;
// }

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

    std::cout << "Enter Server Address: ";
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

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);
        std::string msg = std::string(buffer);

        if (msg == "/exit") {
            break;
        }
        
        if (strcmp("", buffer) == 0) {
            continue;
        }
        
        if (msg.length() > BUFFER_SIZE) {
            std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
            continue;
        }

        // if user types /ping ignore the message.
        // since this keywod serves as a heartbeat mechanism message
        if (strcmp("/ping", buffer) == 0) {
            continue;
        }

        if (send(sock, buffer, sizeof(buffer), 0) == -1) {
            std::cerr << "Could not send message to server: " << ipAddress << std::endl;
            break;
        }

    }

    close(sock);
    return 0; 
}
