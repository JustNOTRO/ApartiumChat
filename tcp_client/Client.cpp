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

Client::Client(std::string name, int socket) name(name), socket(socket) {}

void Client::communicate(std::mutex& clientSockMtx) {
    std::string clientName = this->name;
    char buffer[1024];

    while (true) {
        ssize_t received = recv(this->socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            std::cout << clientName << " disconnected from the server." << std::endl;
            break;
        }

        if (received < 0) {
            std::cerr << "Error receiving data." << std::endl;
            break;
        }

        buffer[received] = '\0';
        this->server.broadcast(this, buffer, clientSockMtx);
    }

    {
        std::lock_guard<std::mutex> lock(clientSockMtx);
        // clientSockets.erase(clientSocket);
        ServerManager::getInstance().removeClient(this->socket);
    }

    close(this->socket);
}

std::string Client::getName() {
    return this->name;
}

int Client::getSocket() {
    return this->socket;
}

void Client::setServer(Server& server) {
    this->server = server;
}
