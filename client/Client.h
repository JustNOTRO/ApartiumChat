//
// Created by notro on 1/28/25.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <cstdlib>

#include "../server/Server.h"


class Client {

public:
    Server server;
    int id;

    bool isConnected() {
        return server.getClients().contains(id);
    }

    int generateId() {
        srand(0);
        rand();
    }
};


#endif //CLIENT_H
