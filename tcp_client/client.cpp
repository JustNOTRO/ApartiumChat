//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "ServerManager.h"
class ServerManager;

int main() {
    const short SERVER_PORT = 8080; // hard coded
    const int& sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Could not connect to server";
        return 1;
    }

    std::cout << "Enter username: ";
    char buffer[1024];
    std::cin.getline(buffer, 1024);

    const std::string& username = buffer;
    const std::string joinMessage = username + " connected to the server.";

    if (send(sock, joinMessage.c_str(), joinMessage.size(), 0) == -1) {
        std::cerr << "Error: Could not send join message." << std::endl;
        close(sock);
        return 1;
    }

    ServerManager& serverManager = ServerManager::getInstance();
    serverManager.addClient(username, sock);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        std::cout << "Enter message: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (send(sock, buffer, sizeof(buffer), 0) == -1) {
            std::cerr << "Error: Could not send message." << std::endl;
            break;
        }
    }

    serverManager.removeClient(sock);
    close(sock);
    return 0;
}
