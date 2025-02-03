//
// Created by notro on 2/1/25.
//

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "Client.h"

class ServerManager {
public:
    static ServerManager& getInstance();

    void addClient(const std::string& username, const int& socket);

    void removeClient(const int& socket);
    
    void cleanupClients();

    Client* getClient(int& socket);

    std::vector<int> getClientSockets();

private:
    ServerManager();
    static std::mutex mtx;
    std::unordered_map<int, Client*> clients;
};

#endif // SERVERMANAGER_H
