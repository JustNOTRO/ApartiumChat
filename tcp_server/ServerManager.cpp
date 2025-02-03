#include <iostream>
#include <vector>

#include "ServerManager.h"
#include "Client.h"

ServerManager::ServerManager() {}

ServerManager& ServerManager::getInstance() {
  std::lock_guard<std::mutex> lock(mtx);
  static ServerManager instance;
  return instance;
}

void ServerManager::addClient(const std::string& username, const int& sock) {
  std::lock_guard<std::mutex> lock(mtx);
  Client* client = new Client(username, sock);
  clients[sock] = client;
}

void ServerManager::removeClient(const int& sock) {
  std::lock_guard<std::mutex> lock(mtx);
  auto it = clients.find(sock);
  if (it == clients.end()) {
      return;
  }

  delete it->second;
  clients.erase(sock);
}

void ServerManager::cleanupClients() {
  for (auto& pair : clients) {
     delete pair.second;
  }

  clients.clear();
}

Client* ServerManager::getClient(int& sock) {
  std::lock_guard<std::mutex> lock(mtx);
  auto it = clients.find(sock);
  if (it != clients.end()) {
      return it->second;
  }

  return nullptr;
}

std::vector<int> ServerManager::getClientSockets() {
  std::lock_guard<std::mutex> lock(mtx);
  std::vector<int> clientSockets;

  for (auto& pair : clients) {
      clientSockets.push_back(pair.first);
  }

  return clientSockets;
}

std::mutex ServerManager::mtx;

