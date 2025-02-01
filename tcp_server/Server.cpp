#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Client.h"

Server::Server(int port) {
    this->port = port;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    this->socket = sock;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->address = serverAddress;

    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Binding failed." << std::endl;
        exit(EXIT_FAILURE);
    }

}

bool Server::connect(int amountOfClients) {
    return listen(this->socket, amountOfClients) >= 0;
}

void Server::close() {
    close(this->socket);
}

void Server::broadcast(Client& client, char buffer[], std::mutex& clientSockMtx) {
    std::string clientName = client.getName();
    int clientSocket = client.getSocket();

    std::string message = clientName + ": " + std::string(buffer);
    std::cout << message << std::endl; // messaging server

    {
        std::lock_guard<std::mutex> lock(clientSockMtx);
        for (int otherSocket : clientSockets) {
            if (otherSocket != clientSocket) {
                send(otherSocket, message.c_str(), message.length(), 0);
            }
        }
    }
}

int Server::getSocket() {
    return this->socket;
}

sockaddr_in getAddress() {
    return this->address;
}