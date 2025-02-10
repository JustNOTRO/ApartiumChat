#include <iostream>
#include <vector>
#include <unordered_map>

#include "ServerManager.h"
#include "Server.h"
#include "Client.h"
ServerManager::ServerManager() {}

ServerManager& ServerManager::getInstance() {
  //std::lock_guard<std::mutex> lock(mtx);
  static ServerManager instance;
  return instance;
}

void ServerManager::addServer(const short& port, Server* server) {
  servers.insert(std::make_pair(port, server));
  std::cout << "Server is running on port " << port << "..." << std::endl;
}

void ServerManager::removeServer(const short& port) {
  auto it = servers.find(port);
  if (it == servers.end()) {
      return;
  }

  delete it->second;
  servers.erase(port);
}

void ServerManager::cleanupServers() {
  for (auto& pair : servers) {
     delete pair.second;
  }

  servers.clear();
}

bool ServerManager::contains(const short& port) {
  return servers[port] != nullptr;
}

/*
  std::lock_guard<std::mutex> lock(mtx);
  auto it = servers.find(port);
  if (it != servers.end()) {
      return it->second;
  }

  return nullptr;
*/

Server* ServerManager::getServer(const short& port) {
  return servers[port];
}

Server* ServerManager::getOrCreateServer(const short& port, size_t numThreads) {
  Server* server = getServer(port);

  if (server == nullptr) {
      server = new Server(port, numThreads);
      addServer(port, server);
      std::cout << "Created new server on port" << port << std::endl;
  }

  return server;
}

std::unordered_map<short, Server*>& ServerManager::getServers() {
  return servers;
}

//std::mutex ServerManager::mtx;
//std::unordered_map<std::string, Client*> ServerManager::clients;

