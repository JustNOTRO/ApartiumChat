//
// Created by notro on 1/28/25.
//

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <functional>

#include "ServerConstants.h"
#include "Client.h"
#include "ThreadPool.h"
#include "NetworkUtils.h"
#include "Logger.h"

class Client;
class ThreadPool;

/**
 * @brief A class representation of the server-side application.
 */
class Server {
    public:
        /**
         * @brief Constructs Server object.
         * @param ip the server IP address
         * @param port the server port
         * @return a new server object
         */
        Server(std::string ip, const std::uint16_t &port);

        /**
         * @brief Destroys the Server object.
         */
        ~Server();

        /**
         * @brief Starts to run the server.
         */
        void run();

        /**
         * @brief Broadcasts the sender's message to all other connected clients.
         * @param senderName the sender's name
         * @param senderSock the sender's socket
         */
        void broadcast(std::string senderName, Socket senderSock);

        /**
         * @brief Adds the client to the server map.
         * @param username the client name
         * @param sock the client socket
         * @return true if client was successfully added, false otherwise
         */
        bool addClient(const std::string &username, Socket sock);

        /**
         * @brief Removes the client from the server map.
         * @param username the client's name
         */
        void removeClient(const std::string &username);
    
        /**
         * @brief Removes all clients from the server map.
         */
        void cleanupClients();

        /**
         * @brief Checks if there is already client with that name in the server.
         * @param username the client's name
         * @return true if there is a client with that name, false otherwise
         */
        bool hasClientWithName(const std::string &username);

        /**
         * @brief Gets all the clients that are currently connected to the server.
         * @return the clients connected to the server
         */
        std::unordered_map<std::string, Client*> &getClients();

        /**
         * @brief Gets the client with name.
         * @param username client name
         * @return the client with that name, null otherwise
         */
        Client* getClient(const std::string &username);

        /**
         * @brief Announces client disconnection.
         * @param username the client name
         */
        void announceUserQuit(const std::string &username, int senderSock);

        /**
         * @brief Sends message to all connected clients.
         * @see broadcast(const std::string, Socket)
         */
        void broadcastMessage(const std::string &username, Socket excludeSock);

        /**
         * @brief Gets the server socket.
         * @return the server socket
         */
        Socket getSocket();

        /**
         * @brief Retrieves the server's address information.
         * @see sockaddr_in
         * @return A sockaddr_in representing the server's address
         */
        sockaddr_in getAddress();


    private:
        std::unordered_map<std::string, Client *> clients;
        ThreadPool threadPool;

        sockaddr_in address;
        Socket sock;
        std::uint16_t port;

};

#endif //SERVER_H
