#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Server.h"
#include "ServerManager.h"
#include "Client.h"

class Server;
class ServerManager;
class Client;

int main() {
    std::cout << "Enter server port: ";
    short port;
    std::cin >> port;

    size_t numThreads = 4;
    ServerManager& serverManager = ServerManager::getInstance();
    Server* server = serverManager.getOrCreateServer(port, numThreads);
    
    sockaddr_in address = server->getAddress();
    socklen_t sockAddrLen = sizeof(address);

    while (true) {
        int clientSocket = accept(server->getSocket(),  (struct sockaddr *)&address, &sockAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Could not accept client socket." << std::endl;
            continue;
        }

        char buffer[1024];
        int received = recv(clientSocket, buffer, sizeof(buffer), 0);
        buffer[received] = '\0';
    
        std::string senderName = std::string(buffer);
        if (server->hasClientWithName(senderName)) {
            send(clientSocket, "Username already taken, Please try other name.", 53, 0);
            continue;
        }

        std::cout << senderName << " connected to the server." << std::endl;
        server->addClient(senderName, clientSocket);

        server->enqueue([&server, senderName, clientSocket] {
            server->broadcast(senderName, clientSocket);
        });
    
    }

    server->disconnect();
    return 0;
}