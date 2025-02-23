//
// Created by notro on 1/28/25.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <cstdlib>
#include <iostream>
#include <mutex>

class Client {

    public:
        Client(std::string name, int sock);

        ~Client();

        std::string getUserName();

        int getSocket();

    private:
        std::string name;
        int sock;
    
};


#endif //CLIENT_H
