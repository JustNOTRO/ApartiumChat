//
// Created by notro on 1/28/25.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <cstdlib>

#include "Server.h"

class Server;

class Client {

    public:
        Client(std::string name, int socket);

        std::string getName();

        int getSocket();

        void setServer(Server& server);

        void communicate(std::mutex& clientSockMtx);

    private:
        Server* server;
        std::string name;
        int socket;
    
};


#endif //CLIENT_H
