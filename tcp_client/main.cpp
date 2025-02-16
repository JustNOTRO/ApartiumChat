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

#define DEFAULT_PORT 3333
#define BUFFER_SIZE 2000

short getSelectedPort(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    if (colonPos == std::string::npos) {
        return DEFAULT_PORT;
    }

    // Parse the port as short since we don't need that many bytes.
    short port = static_cast<short>(std::stoi(ipAddress.substr(colonPos + 1)));
    return port;
}

std::string getSelectedIpAddress(std::string& providedAddress) {
    size_t colonPos = providedAddress.find(':');
    return providedAddress.substr(0, colonPos);
}

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

bool connectToServer(std::string& ipAddress, int sock) {
    const short& port = getSelectedPort(ipAddress);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    const std::string& ip = getSelectedIpAddress(ipAddress);

    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP Address: " << ipAddress << std::endl;
        return false;
    }

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

int main() {
    const int& sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    std::cout << "Enter Server Address: ";
    std::string ipAddress;
    std::cin >> ipAddress;

    if (!connectToServer(ipAddress, sock)) {
        std::cerr << "Could not connect to server: " << ipAddress << std::endl;
        return 1;
    }

    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;

    send(sock, username.c_str(), username.length(), 0); // sending username

    // Spawn a thread for handling incoming messages
    std::thread(handleIncomingMessages, sock).detach();

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);

        if (strcmp("/exit", buffer) == 0) {
            break;
        }
        
        if (strcmp("", buffer) == 0) {
            continue;
        }

        std::string msg = std::string(buffer);
        if (msg.length() > BUFFER_SIZE) {
            std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
            continue;
        }

        std::string prefixedMsg = username + ": " + msg;
        if (send(sock, prefixedMsg.c_str(), prefixedMsg.length(), 0) == -1) {
            std::cerr << "Could not send message to server: " << ipAddress << std::endl;
            break;
        }
    }

    close(sock);
    return 0; 
}
