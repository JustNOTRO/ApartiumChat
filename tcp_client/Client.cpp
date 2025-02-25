//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Client.h"

Client::Client(std::string name, Socket sock) : name(name), sock(sock) {}

Client::~Client() {}

std::string Client::getUserName() {
    return this->name;
}

Socket Client::getSocket() {
    return this->sock;
}

