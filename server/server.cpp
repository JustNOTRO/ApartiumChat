#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unordered_map>
#include <arpa/inet.h>

int main() {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::perror("Could not create socket");
        return -1;
    }

    sockaddr_in sockAddress{};
    sockAddress.sin_family = AF_INET;
    sockAddress.sin_port = htons(8080);
    sockAddress.sin_addr.s_addr = INADDR_ANY;

    if (inet_pton(AF_INET, "127.0.0.1", &sockAddress.sin_addr) == -1) {
        std::perror("Invalid IP Address");
        return -2;
    }

    socklen_t sockAddrLen = sizeof(sockAddress);

    if (bind(sock, reinterpret_cast<sockaddr *>(&sockAddress), sockAddrLen) == -1) {
        std::perror("Could not bind socket");
        return -3;
    }

    if (listen(sock, 5) == -1) {
        std::perror("Could not listen on socket");
        return -4;
    }

    char buffer[1024];
    while (true) {
        memset(buffer, 0, 1024);
        const int client = accept(sock, reinterpret_cast<struct sockaddr*>(&sockAddress), &sockAddrLen);

        std::cout << "Stage 1" << std::endl;
        if (client < 0) {
            std::perror("accept");
            std::cout << "Exit on client < 0" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Stage 2" << std::endl;

        int received = recv(client, buffer, sizeof(buffer), 0);
        if (received == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        std::cout << "Stage 3" << std::endl;
        std::cout << "Received message from client: " << buffer << std::endl;
        send(client, "Hello from server!", 18, 0);
    }

    std::cout << "Stage 4" << std::endl;

    close(sock);
    std::cout << "Closed socket" << std::endl;
    return 0;
}
