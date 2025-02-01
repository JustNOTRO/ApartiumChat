#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include "Client.h"

class Client;

class ServerManager {
    public:
        static ServerManager& getInstance();

        void addClient(std::string username, int socket);

        void removeClient(int socket);

        Client& getClient(int socket);


    private:
        ServerManager();

        static ServerManager& instance;
        std::unordered_map<int, Client> clients;
}

#endif //SERVER_MANAGER_H