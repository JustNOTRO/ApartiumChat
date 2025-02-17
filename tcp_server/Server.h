//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <list>
#include <set>
#include <unordered_map>
#include <netinet/in.h>
#include <functional>

#include "Client.h"
#include "ThreadPool.h"

class Client;
class ThreadPool;

class Server {
    public:
        Server(std::string ip, const short& port);

        ~Server();

        void run();

        void broadcast(std::string senderName, int senderSock);

        void sendHeartbeatToSender(const int& sock);

        bool addClient(const std::string& username, const int& sock);

        void removeClient(const std::string& username);
    
        void cleanupClients();

        bool hasClientWithName(const std::string& username);

        std::unordered_map<std::string, Client*>& getClients();

        std::vector<int> getClientSockets();

        Client* getClient(const std::string& username);

        void announceUserQuit(const std::string& username);

        void disconnect();

        int getSocket();

        sockaddr_in getAddress();

    private:
        std::unordered_map<std::string, Client*> clients;

        ThreadPool threadPool;
        sockaddr_in address;
        int amountOfClients;
        int sock;
        short port;

};

#endif //SERVER_H
