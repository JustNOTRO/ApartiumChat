//
// Created by notro on 2/1/25.
//

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "Client.h"
#include "Server.h"

class ServerManager {
public:
    static ServerManager& getInstance();

    void addServer(const short& port, Server* server);

    void removeServer(const short& port);
    
    void cleanupServers();

    bool contains(const short& port);

    Server* getServer(const short& port);

    Server* getOrCreateServer(const short& port, size_t numThreads);

    std::unordered_map<short, Server*>& getServers();

private:
    ServerManager();
    std::unordered_map<short, Server*> servers;
};

#endif // SERVERMANAGER_H
