//
// Created by notro on 2/1/25.
//

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <unordered_map>
#include <string>


class Client;

class ServerManager {
public:
    static ServerManager& getInstance();  // Singleton

    void addClient(std::string username, int socket);

    void removeClient(int socket);

    Client& getClient(int socket);

private:
    ServerManager();

    std::unordered_map<int, Client> clients;
};

#endif // SERVERMANAGER_H
