//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "ServerManager.h"
#include "Client.h"

Client::Client(std::string name, int sock) : name(name), sock(sock) {}

std::string Client::getName() {
    return this->name;
}

int Client::getSocket() {
    return this->sock;
}

void Client::setServer(Server& server) {
    this->server = &server;
}

void Client::communicate() {
    std::string clientName = this->name;
    char buffer[1024];

    while (true) {
        ssize_t received = recv(this->sock, buffer, sizeof(buffer), 0);
        if (received == 0) {
            std::cout << clientName << " disconnected from the server." << std::endl;
            break;
        }

        if (received < 0) {
            std::cerr << "Error receiving data." << std::endl;
            break;
        }

        buffer[received] = '\0';
        this->server->broadcast(this, buffer);
    }

    ServerManager::getInstance().removeClient(this->sock);
    close(this->sock);
}

