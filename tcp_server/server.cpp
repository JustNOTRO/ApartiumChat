#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Server.h"
#include "Client.h"
#include "ServerManager.h"

std::mutex clientSocketsMtx;

int main() {
    const short SERVER_PORT = 8080; // hard coded
    Server server(SERVER_PORT);

    int amountOfClients = 5; // todo remove hardcoded value
    if (!server.connect(amountOfClients)) {
        std::cerr << "Could not connect to server.";
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running on port " << SERVER_PORT << "..." << std::endl;

    ServerManager& serverManager = ServerManager::getInstance();
    sockaddr_in address = server.getAddress();
    socklen_t sockAddrLen = sizeof(address);

    while (true) {
        int clientSocket = accept(server.getSocket(),  (struct sockaddr *)&address, &sockAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Could not accept client socket." << std::endl;
            continue;
        }

        Client& client = serverManager.getClient(clientSocket);
        {
            std::lock_guard<std::mutex> lock(clientSocketsMtx);
            // clientSocket.insert(clientSocket);
            client.setServer(server);

            std::cout << client.getName() << " connected to the server." << std::endl;
        }

        // Spawn a new thread to handle the client communication
        std::thread([&client](){ client.communicate(clientSocketsMtx); }).detach();
    }

    server.close();
    // close(sock);
    return 0;
}