//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Client;

class Server {
    public:
        explicit Server(int port);

        ~Server(); // TODO implement destructor

        bool connect(int amountOfClients);

        void disconnect();

        void broadcast(Client& client, char buffer[], std::mutex& clientSockMtx);

        int getSocket();

        sockaddr_in getAddress();
        

    private:
        sockaddr_in address;
        int sock;
        int port;

};

#endif //SERVER_H
