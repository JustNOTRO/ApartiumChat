//
// Created by notro on 1/28/25.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <cstdlib>
#include <iostream>
#include <mutex>

#include "Server.h"

class Server;

class Client {

    public:
        Client(std::string name, int sock);

        std::string getName();

        int getSocket();

        void setServer(Server& server);

        void communicate();

    private:
        Server* server;
        std::string name;
        int sock;
    
};


#endif //CLIENT_H
