#include <iostream>

#include "ServerManager.h"
#include "Client.h"

ServerManager::ServerManager() {}

ServerManager& ServerManager::getInstance() {
  static ServerManager instance;
  return instance;
}

void ServerManager::addClient(std::string username, int socket) {
  Client client(username, socket);
  clients.insert(socket, client);
}

void ServerManager::removeClient(int socket) {

}

Client& ServerManager::getClient(int socket) {
  return clients.at(socket);
}

