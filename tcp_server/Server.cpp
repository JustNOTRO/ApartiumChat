#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <functional>
#include <unordered_map>

#include "Server.h"
#include "Client.h"
#include "ThreadPool.h"
#include "ServerConstants.h"

#define NUM_THREADS 4

class ThreadPool;

std::mutex mtx;

Server::Server(std::string ip, const short& port) : port(port), threadPool(NUM_THREADS) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    this->sock = sock;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
    this->address = serverAddress;

    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP Address: " << ip << " " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int connectionBacklog = 5;
    if (listen(this->sock, connectionBacklog) == -1) {
        std::cerr << "Could not listen to server." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Created server on IP Address: " << ip << ":" << port << std::endl;
}

Server::~Server() {
    cleanupClients();
    close(this->sock);
}

void Server::run() {
    socklen_t sockAddrLen = sizeof(address);
    struct sockaddr* socketAddress = reinterpret_cast<struct sockaddr*>(&address);

    while (true) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        int clientSocket = accept(sock, socketAddress, &sockAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Could not accept client socket." << std::endl;
            continue;
        }
        
        int received = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (received <= 0) {
            close(clientSocket);
            continue;
        }

        buffer[received] = '\0';
        std::string senderName(buffer);

        if (!addClient(senderName, clientSocket)) {
            close(clientSocket);
            continue;
        }
        
        std::string joinMsg = senderName + " connected to the server.";
        std::cout << joinMsg << std::endl;

        broadcastMessage(joinMsg, clientSocket);
        
        threadPool.enqueue([this, senderName, clientSocket] {
            broadcast(senderName, clientSocket);
        });
    }
}

void Server::broadcast(std::string senderName, int senderSock) {
    char buffer[BUFFER_SIZE];
    std::unordered_map<std::string, Client*>& clients = getClients();

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytesReceived = recv(senderSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived < 0) {
            std::cerr << "Could not receive message from " << senderName << "." << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            announceUserQuit(senderName);
            break;
        }
        
        std::string msg = std::string(buffer);
        if (msg.empty()) { 
            continue;
        }

        if (msg == HEARTBEAT_REQUEST) {
            send(senderSock, HEARTBEAT_RESPONSE, sizeof(HEARTBEAT_RESPONSE), 0);
            continue;
        }

        std::string prefixedMsg = senderName + ": " + msg;
        std::cout << prefixedMsg << std::endl;

        for (auto& pair : clients) {
            std::string username = pair.first;
            Client* client = pair.second;

            if (client == nullptr) {
                std::cerr << "Client is null for username: " << username << std::endl;
                continue;
            }

            int clientSocket = client->getSocket();
            if (clientSocket == senderSock) {
                continue;
            }
            
            if (send(clientSocket, prefixedMsg.c_str(), prefixedMsg.length(), 0) == -1) {
                std::cerr << "Could not send message to Client: " << username << std::endl;
            }
        }
    }

    removeClient(senderName);
    close(senderSock);
}

void Server::broadcastMessage(const std::string &msg, int excludeSock = -1) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &pair : clients) {
        int clientSocket = pair.second->getSocket();

        if (clientSocket == excludeSock) {
            continue;
        }

        if (send(clientSocket, msg.c_str(), msg.length(), 0) == -1) {
            std::cerr << "Could not send message to " << pair.first << std::endl;
        }
    }
}

bool Server::addClient(const std::string& username, const int& sock) {
    std::lock_guard<std::mutex> lock(mtx);
  
    if (hasClientWithName(username)) {
        send(sock, USERNAME_TAKEN_MSG, sizeof(USERNAME_TAKEN_MSG), 0);
        return false;
    }
  
    Client* client = new Client(username, sock);
    clients[username] = client;
    return true;
}

void Server::removeClient(const std::string& username) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = clients.find(username);
    if (it == clients.end()) {
        return;
    }
  
    delete it->second;
    clients.erase(username);
}

void Server::cleanupClients() {
    std::lock_guard<std::mutex> lock(mtx);
  
    for (auto& pair : clients) {
       delete pair.second;
    }
  
    clients.clear();
}

bool Server::hasClientWithName(const std::string& username) {
    return clients.find(username) != clients.end();
}

std::unordered_map<std::string, Client*>& Server::getClients() {
    std::lock_guard<std::mutex> lock(mtx);
    return clients;
}

std::vector<int> Server::getClientSockets() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<int> clientSockets;
  
    for (auto& pair : clients) {
        Client* client = pair.second;
        clientSockets.push_back(client->getSocket());
    }
  
    return clientSockets;
}

Client* Server::getClient(const std::string& username) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = clients.find(username);
    if (it != clients.end()) {
        return it->second;
    }
  
    return nullptr;
}

int Server::getSocket() {
    return this->sock;
}

sockaddr_in Server::getAddress() {
    return this->address;
}

void Server::announceUserQuit(const std::string& username) {
    std::string quitMsg = username + " disconnected from the server.";
    std::cout << quitMsg << std::endl;

    if (clients.empty()) {
        return;
    }

    for (auto& pair : clients) {
        Client* client = pair.second;
        if (send(client->getSocket(), quitMsg.c_str(), quitMsg.length(), 0) == -1) {
            std::cerr << "Could not send quit message of " << username << " to: " << pair.first << std::endl;
        }
    }
}






