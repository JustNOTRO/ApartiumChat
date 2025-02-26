//
// Created by notro on 1/28/25.
//

#ifndef CLIENT_H
#define CLIENT_H

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <cstring>

#include "SocketAdapter.h"

/**
 * @brief A class representation of the client-side application.
 */
class Client {

    public:
        /**
         * @brief Constructs new Client.
         * @param name the client name
         * @param sock the client socket
         */
        Client(std::string name, Socket sock);

        /**
         * @brief Destroys the Client.
         */
        ~Client();

        /**
         * @brief Gets the client's name.
         * @return the client name
         */
        std::string getUserName();

        /**
         * @brief Gets the client's socket.
         * @return client socket
         */
        Socket getSocket();

    private:
        std::string name;
        Socket sock;
    
};


#endif //CLIENT_H
