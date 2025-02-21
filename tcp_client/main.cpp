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
#include "ServerConstants.h"

std::vector<std::string> servers = std::vector<std::string>();

bool sendHeartbeat(const int &sock) {
    if (send(sock, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) < 0) {
        std::cerr << "Could not send heartbeat: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

bool isServerResponding(const int &sock, char buffer[]) {
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);

    if (bytesReceived > 0)
        return true;

    if (bytesReceived < 0) {
        std::cerr << "Error while trying to receive data from server: " << strerror(errno) << std::endl;
    }

    std::cerr << "Server is not responding.. disconnecting.." << std::endl;
    return false;
}

void handleIncomingMessages(const int &sock) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        struct timeval timeout;
        timeout.tv_sec = HEARTBEAT_INTERVAL_SECONDS;
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
        sleep(HEARTBEAT_INTERVAL_SECONDS); // Wait 5 seconds for response

        if (!isServerResponding(sock, buffer)) {
            break;
        }

        if (strncmp(HEARTBEAT_RESPONSE, buffer, sizeof(HEARTBEAT_RESPONSE)) != 0) {
            std::cout << buffer << std::endl;
        }

    }

    close(sock);
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

    bool connectionEstablished = connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
    if (connectionEstablished) {
        std::thread(handleIncomingMessages, sock).detach();
    }

    return connectionEstablished;
}

struct FallbackServer {
    std::string *ip = nullptr;
    int id;

    bool connect() {
        for (int i = 0; i < servers.size(); ++i) {
            int sock = createSocket();
            std::string ip = servers[i];
    
            if (connectToServer(ip, sock) == 0) {
                this->ip = &ip;
                this->id = i + 1;
                std::cout << "Connection to Server #" << id << ": " << ip << " has been established." << std::endl;
                break;
            }
    
            close(sock); // if connection was not established, close the socket so he wont remain open 
            servers.erase(servers.begin() + i);
        }

        if (ip == nullptr) {
            servers.clear();
            return false;
        }

        return true;
    }
};

bool connectToFallbackServer() {
    FallbackServer fallbackServer;
    return fallbackServer.connect();
}

int createSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main() {
    const int &sock = createSocket();

    std::cout << "Enter IP Addresses by order, the first server will be the first you will connect to." << std::endl;
    std::cout << "If an unexpected server crash occurs, the client will behave the next server as fallback server.";
    std::cout << "Invoke /done when you want to terminate the list." << std::endl;
    
    std::string ipAddress;
    do {
        int serverId = servers.size() + 1;
        std::cout << "Enter #" << serverId << " IP Address: " << std::endl;
        std::cin >> ipAddress;
        
        servers.push_back(ipAddress);
        std::cin.clear();

        if (servers.size() == MAX_FALLBACK_SERVERS) {
            std::cout << "Maximum allowed fallback servers reached. terminating process..";
            break;
        }
    } while (ipAddress != "/done");


    if (!connectToServer(ipAddress, sock)) {
        std::cerr << "Could not connect to server: " << ipAddress << std::endl;
        close(sock);

        if (!connectToFallbackServer()) {
            std::cerr << "Failed to connect to fallback servers " << strerror(errno) << std::endl;
            return 1;
        }
    }

    std::cin.clear();
    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;
    send(sock, username.c_str(), username.length(), 0); // Sending username

    //std::thread(handleIncomingMessages, sock).detach();

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);
        std::string msg = std::string(buffer);

        size_t msgLen = msg.length();
        if (msgLen == 0) {
            continue;
        }

        if (msgLen > BUFFER_SIZE) {
            std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
            break;
        }

        if (msg == "/exit") {
            std::cout << "Disconnected from server: " << ipAddress << std::endl;
            close(sock);
            break;
        }

        // Ignore client attempts to send a heartbeat
        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(sock, buffer, BUFFER_SIZE, 0) == -1) {
            std::cerr << "Could not send message to server: " << ipAddress << std::endl;
            continue;
        }

    }

    return 0;
}