//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_map>

#include "Client.h"

class Client;

class Server {
    public:
        Server(int port);

        ~Server(); // TODO implement destructor

        bool connect(int amountOfClients);

        void close();

        void broadcast(Client& client, char buffer[], std::mutex& clientSockMtx);

        int getSocket();

        sockaddr_in getAddress();

        

    private:
        sockaddr_in address;
        int socket;
        int port;

};

#endif //SERVER_H
