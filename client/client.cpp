//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>

int main() {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Could not connect to server";
        return -1;
    }

    std::cout << "Enter username: ";
    char buffer[1024];
    std::cin.getline(buffer, 1024);

    const std::string username = buffer;
    const std::string joinMessage = username + " connected to the server.";

    send(sock, joinMessage.c_str(), joinMessage.size(), 0);

    while (true) {
        memset(buffer, 0, 1024);
        std::cout << "Enter message: ";
        std::cin.getline(buffer, 1024);

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
