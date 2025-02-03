//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <list>
#include <set>
#include <unordered_map>
#include <netinet/in.h>

#include "Client.h"

class Client;

class Server {
    public:
        Server(const short& port);

        ~Server(); // TODO implement destructor

        bool connect(int amountOfClients);

        void disconnect();

        void broadcast(Client* client, char buffer[]);

        int getSocket();

        sockaddr_in getAddress();

        

    private:
        sockaddr_in address;
        int sock;
        int port;

};

#endif //SERVER_H
