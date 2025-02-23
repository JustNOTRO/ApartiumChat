#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "utils/ServerUtils.h"
#include "ServerConstants.h"
#include "Server.h"

std::vector<std::string> servers;
std::mutex serverMutex;
std::atomic<int> sock;

int createSocket() {
    int newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (newSock < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    return newSock;
}

bool sendHeartbeat(int sockFD) {
    if (send(sockFD, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) == -1) {
        std::cerr << "Could not send heartbeat: " << strerror(errno) << std::endl;
        return false;
    }

    return true; 
}

bool isServerResponding(int sockFD, int &bytesReceived) {
    if (bytesReceived > 0) {
        return true;
    }

    if (bytesReceived < 0) {
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
    }

    std::cerr << "Server is not responding.. disconnecting.." << std::endl;
    return false;
}

bool connectToServer(const std::string &ipAddress, int sock) {
    std::string ip = ServerUtils::getSelectedIpAddress(ipAddress);
    short port = ServerUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP Address: " << strerror(errno) << std::endl;
        return false;
    }
    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

struct FallbackServer {
private:
    std::string ipAddress;
    int newSock;

public:
    std::string getIpAddress() {
        return ipAddress;
    }

    int getSocket() {
        return newSock;
    }

    bool connect() {
        for (const auto &server : servers) {
            int tempSock = createSocket();
            if (connectToServer(server, tempSock)) {
                newSock = tempSock;
                ipAddress = server;
                return true;
            }

            close(tempSock);
        }
        return false;
    }
};

void handleIncomingMessages(const std::string &username) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        struct timeval timeout = {HEARTBEAT_INTERVAL_SECONDS, 0};
        fd_set readSocks;
        FD_ZERO(&readSocks);
        FD_SET(sock.load(), &readSocks);

        int readySocks = select(sock.load() + 1, &readSocks, nullptr, nullptr, &timeout);
        if (readySocks < 0) {
            std::cerr << "Error during select(): " << strerror(errno) << ". Disconnecting..." << std::endl;
            break;
        }

        if (readySocks == 0) {
            sendHeartbeat(sock.load());
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        int bytesReceived = recv(sock.load(), buffer, BUFFER_SIZE, 0);
        if (isServerResponding(sock.load(), bytesReceived)) {
            if (strcmp(buffer, USERNAME_TAKEN_MSG) == 0) {
                std::cout << buffer << std::endl;
                break;
            }

            if (strncmp(buffer, HEARTBEAT_RESPONSE, sizeof(HEARTBEAT_RESPONSE)) != 0) {
                std::cout << buffer << std::endl;
            }
            continue;
        }

        FallbackServer fallbackServer;
        if (!fallbackServer.connect()) {
            std::cerr << "Could not connect to fallback servers: " << strerror(errno) << std::endl;
            break;
        }

        sock.store(fallbackServer.getSocket());
        send(sock.load(), username.c_str(), username.length(), 0);
        std::cout << "Connected to fallback server: " << fallbackServer.getIpAddress() << std::endl;
    }

    close(sock.load());
}

void handleClientInput() {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);
        std::string msg(buffer);
        
        if (msg.empty()) {
            continue;
        }
        
        if (msg.length() > BUFFER_SIZE) {
            std::cerr << "Message too long." << std::endl;
            continue;
        }
        
        if (msg == EXIT_CMD) {
            servers.clear();
            close(sock.load());
            break;
        }
        
        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(sock.load(), msg.c_str(), msg.length(), 0) == -1) {
            std::cerr << "Could not send message: " << strerror(errno) << std::endl;
        }
    }
}

int main() {
    sock.store(createSocket());

    std::string ipAddress;
    std::cout << "Enter IP Addresses: (type /done to finish):" << std::endl;
    while (true) {
        std::cin >> ipAddress;
        if (ipAddress == "/done") {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(serverMutex);
            servers.push_back(ipAddress);
        }
    }
    
    if (servers.empty()) {
        std::cerr << "No servers provided." << std::endl;
        return 1;
    }
    
    if (!connectToServer(servers[0], sock.load())) {
        std::cerr << "Could not connect to server: " << servers[0] << std::endl;
        FallbackServer fallbackServer;
        if (!fallbackServer.connect()) {
            std::cerr << "Could not connect to fallback servers." << std::endl;
            return 1;
        }
        sock.store(fallbackServer.getSocket());
        std::cout << "Connected to fallback server: " << fallbackServer.getIpAddress() << std::endl;
    }
    
    std::string username;
    while (username.empty()) {
        std::cout << "Enter username: ";
        std::cin >> username;
    }
    
    send(sock.load(), username.c_str(), username.length(), 0);

    std::thread(handleIncomingMessages, username).detach();
    handleClientInput();
    return 0;
}
