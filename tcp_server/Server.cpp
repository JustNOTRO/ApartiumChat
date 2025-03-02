#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <functional>
#include <unordered_map>

#include "Server.h"

#define NUM_THREADS 4
std::mutex mtx;

Server::Server(std::string ip, const std::uint16_t &port) : port(port), threadPool(NUM_THREADS) {
    Socket sock = NetworkUtils::createSocket();
    this->sock = sock;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    this->address = serverAddress;

    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP Address: '" << ip << "'." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Bind failed " << NetworkUtils::getLastError() << "." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int connectionBacklog = 5;
    if (listen(this->sock, connectionBacklog) == -1) {
        std::cerr << "Could not listen to server: " << NetworkUtils::getLastError() << "." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Created server on IP Address: " << ip << ":" << port << std::endl;
}

Server::~Server() {
    cleanupClients();
    NetworkUtils::closeSocket(this->sock);

    #ifdef _WIN32
        WSACleanup();
    #endif
}

void Server::run() {
    char buffer[BUFFER_SIZE];
    socklen_t sockAddrLen = sizeof(address);
    struct sockaddr* socketAddress = reinterpret_cast<struct sockaddr*>(&address);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        Socket clientSocket = accept(sock, socketAddress, &sockAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Could not accept client socket: " << NetworkUtils::getLastError() << "." << std::endl;
            continue;
        }
        
        int bytesRecieved = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        buffer[bytesRecieved] = '\0';

        std::string senderName(buffer);
        if (bytesRecieved < 0) {
            std::cerr << "Could not receive data from " << senderName << ":" << NetworkUtils::getLastError() << "." << std::endl;
            continue;
        }

        if (senderName.empty()) {
            continue;
        }

        if (!addClient(senderName, clientSocket)) {
            send(clientSocket, USERNAME_TAKEN_MSG, sizeof(USERNAME_TAKEN_MSG), 0);
            NetworkUtils::closeSocket(clientSocket);
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

void Server::broadcast(std::string senderName, Socket senderSock) {
    char buffer[BUFFER_SIZE];
    std::unordered_map<std::string, Client*> &clients = getClients();

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytesReceived = recv(senderSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived < 0) {
            std::cerr << "Could not receive message from " << senderName << "." << std::endl;
            break;
        }

        if (bytesReceived == 0) {
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
        broadcastMessage(prefixedMsg, senderSock);
    }
    
    removeClient(senderName);
    announceUserQuit(senderName, senderSock);
    NetworkUtils::closeSocket(senderSock);
}

void Server::broadcastMessage(const std::string &msg, Socket excludeSock) {
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

bool Server::addClient(const std::string &username, Socket sock) {
    std::lock_guard<std::mutex> lock(mtx);
  
    if (hasClientWithName(username)) {
        return false;
    }
  
    Client *client = new Client(username, sock);
    clients[username] = client;
    return true;
}

void Server::removeClient(const std::string &username) {
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

bool Server::hasClientWithName(const std::string &username) {
    return clients.find(username) != clients.end();
}

std::unordered_map<std::string, Client *>& Server::getClients() {
    std::lock_guard<std::mutex> lock(mtx);
    return clients;
}

void Server::announceUserQuit(const std::string &username, int senderSock) {
    std::string quitMsg = username + " disconnected from the server.";
    std::cout << quitMsg << std::endl;

    broadcastMessage(quitMsg, senderSock);
}