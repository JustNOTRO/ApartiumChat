#include <iostream>

#include "Client.h"

class ServerManager {

    ServerManager::ServerManager() {}

    ServerManager&::getInstance() {
        return !instance ? (instance = ServerManager::ServerManager()) : instance;
    }

    ServerManager::clients = std::unordered_map<int, Client>();

    void ServerManager::addClient(std::string username, int socket) {
        Client client(username, socket);
        clients.insert(client);
    }

    void ServerManager::removeClient(int socket) {
        clients.erase(socket);
    }

    Client& ServerManager::getClient(int socket) {
        return clients.at(socket);
    }
}