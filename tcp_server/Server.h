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
        Server(const short& port, size_t numThreads);

        ~Server();

        void disconnect();

        void broadcast(std::string senderName, int senderSock);

        void enqueue(std::function<void()> task);

        int getSocket();

        sockaddr_in getAddress();

        void addClient(const std::string& username, const int& sock);

        void removeClient(const std::string& username);
    
        void cleanupClients();

        bool hasClientWithName(const std::string& username);
        
        void announceUserQuit(const std::string& username);

        Client* getClient(const std::string& username);

        std::vector<int> getClientSockets();

        std::unordered_map<std::string, Client*>& getClients();

    private:
        std::unordered_map<std::string, Client*> clients;

        ThreadPool threadPool;
        sockaddr_in address;
        int amountOfClients;
        int sock;
        short port;

};

#endif //SERVER_H
