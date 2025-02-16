#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <functional>
#include <unordered_map>

#include "Server.h"
#include "Client.h"
#include "ThreadPool.h"

#define NUM_THREADS 4

class ThreadPool;

std::mutex mtx;

Server::Server(const short& port) :
               port(port), threadPool(NUM_THREADS) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    this->sock = sock;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->address = serverAddress;

    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Binding failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    int connectionBacklog = 5;
    if (listen(this->sock, connectionBacklog) == -1) {
        std::cerr << "Could not listen to server." << std::endl;
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {}

void Server::run() {
    socklen_t sockAddrLen = sizeof(address);

    while (true) {
        int clientSocket = accept(sock, (struct sockaddr *)&address, &sockAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Could not accept client socket." << std::endl;
            continue;
        }

        char buffer[1024];
        int received = recv(clientSocket, buffer, sizeof(buffer), 0);
        buffer[received] = '\0';
    
        std::string senderName = std::string(buffer);

        std::cout << senderName << " connected to the server." << std::endl;
        addClient(senderName, clientSocket);

        threadPool.enqueue([this, senderName, clientSocket] {
            broadcast(senderName, clientSocket);
        });
    }

    disconnect();
}

void Server::broadcast(std::string senderName, int senderSock) {
    char buffer[1024];
    std::unordered_map<std::string, Client*>& clients = getClients();

    while (true) {
        size_t bufferSize = sizeof(buffer);
        memset(buffer, 0, bufferSize);

        int bytesReceived = recv(senderSock, buffer, bufferSize, 0);
        if (bytesReceived < 0) {
            std::cerr << "Could not receive message from " << senderName << "." << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            announceUserQuit(senderName);
            break;
        }

        std::cout << buffer << std::endl;

        for (auto& pair : clients) {
            Client* client = pair.second;
            if (client == nullptr) {
                std::cerr << "Client is null for username: " << pair.first << std::endl;
                continue;
            }

            int clientSocket = client->getSocket();
            if (clientSocket == senderSock) {
                continue;
            }
            
            if (send(clientSocket, buffer, bytesReceived, 0) == -1) {
                std::cerr << "Could not send message to Client: " << client->getUserName() << std::endl;
            }
        }
    }

    removeClient(senderName);
    close(senderSock);
}

void Server::addClient(const std::string& username, const int& sock) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string msg = "Username already taken, Please try other name.";
  
    if (hasClientWithName(username)) {
        send(sock, msg.c_str(), msg.length(), 0);
        return;
    }
  
    Client* client = new Client(username, sock);
    clients[username] = client;
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

void Server::disconnect() {
    cleanupClients();
    close(this->sock);
    delete this;
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

    for (auto& pair : clients) {
        Client* client = pair.second;
        if (send(client->getSocket(), quitMsg.c_str(), quitMsg.length(), 0) == -1) {
            std::cerr << "Could not send quit message of " << username << " to: " << pair.first << std::endl;
        }
    }
}






