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
#define BUFFER_SIZE 2000
#define CONNECTION_TIMEOUT 10

class ServerUtils;

void handleIncomingMessages(const int& sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0 && strcmp("/pong", buffer) != 0) {
            std::cout << buffer << std::endl;
        }
    }
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

void sendHeartbeatToServer(const int& sock) {
    time_t lastHeartbeat = time(nullptr);
    char buffer[5];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        send(sock, "/ping", 5, 0);
        std::cout << "Sent ping" << std::endl;

        fd_set set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        timeout.tv_sec = CONNECTION_TIMEOUT;
        timeout.tv_usec = 0;
            
        int ready = select(sock + 1, &set, nullptr, nullptr, &timeout);
        if (ready <= 0) {
            std::cout << "Server is not responding. Disconnecting." << std::endl;
            break;
        }

        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0 && strncmp(buffer, "/pong", 5) == 0) {
            std::cout << "PONG!" << std::endl;
            lastHeartbeat = time(nullptr);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    close(sock);
    exit(EXIT_FAILURE);
}

int main() {
    const int& sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    std::cout << "Enter Server Address: ";
    std::string ipAddress;
    std::cin >> ipAddress;

    if (!connectToServer(ipAddress, sock)) {
        std::cerr << "Could not connect to server: " << ipAddress << "." << std::endl;
        return 1;
    }

    std::cin.clear();
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;

    send(sock, username.c_str(), username.length(), 0); // Sending username

    // Spawning a thread for handling incoming messages
    std::thread(handleIncomingMessages, sock).detach();

    // Spawning a thread for sending heartbeat to server
    std::thread(sendHeartbeatToServer, sock).detach();

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);

        if (strcmp("/exit", buffer) == 0) {
            break;
        }
        
        if (strcmp("", buffer) == 0) {
            continue;
        }

        std::string msg = std::string(buffer);
        if (msg.length() > BUFFER_SIZE) {
            std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
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
