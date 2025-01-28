//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_map>


class Client;

class Server {
private:
    std::unordered_map<int, Client> clients = std::unordered_map<int, Client>();

public:
    std::unordered_map<int, Client> getClients() {
        return this->clients;
    }
};



#endif //SERVER_H
