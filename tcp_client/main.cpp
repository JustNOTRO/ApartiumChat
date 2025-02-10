//
// Created by notro on 1/25/25.
//

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

void handleIncomingMessages(const int& sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << buffer << std::endl;
        }
    }
}

int main() {
    std::cout << "Enter server port: ";
    short port;
    std::cin >> port;

    const int& sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Could not connect to server on port: " << port;
        return 1;
    }

    send(sock, username.c_str(), username.length(), 0); // sending username

    // Spawn a thread for handling incoming messages
    std::thread(handleIncomingMessages, sock).detach();


    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp("/exit", buffer) == 0) {
            break;
        }
        
        if (strcmp("", buffer) == 0) {
            continue;
        }

        std::string msg = username + ": " + std::string(buffer);
        if (send(sock, msg.c_str(), msg.length(), 0) == -1) {
            std::cerr << "Error: Could not send message." << std::endl;
            break;
        }
    }

    close(sock);
    return 0; 
}
