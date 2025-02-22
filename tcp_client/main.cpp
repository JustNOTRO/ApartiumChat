#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include "utils/ServerUtils.h"
#include "ServerConstants.h"
#include "Server.h"

std::mutex serverMutex;

int createSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    return sock;
}

bool sendHeartbeat(const int &sock) {
    if (send(sock, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) < 0) {
        std::cerr << "Could not send heartbeat: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

bool isServerResponding(const int &sock, int bytesReceived) {
    if (bytesReceived > 0) {
        return true;
    }

    if (bytesReceived < 0) {
        std::cerr << "Error while trying to receive data from server: " << strerror(errno) << std::endl;
    }

    std::cerr << "Server is not responding.. disconnecting.." << std::endl;
    return false;
}

bool connectToServer(const std::string &ipAddress, const int &sock) {
    std::cout << "I was here" << std::endl;

    const std::string &ip = ServerUtils::getSelectedIpAddress(ipAddress);
    const short &port = ServerUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP Address " << strerror(errno) << "." << std::endl;
        return false;
    }

    std::cout << strerror(errno) << std::endl;
    std::cout << connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

std::vector<std::string> servers;

struct FallbackServer {
    private:
        std::string *ipAddress = nullptr;
        int sock;

    public:
        std::string getIpAddress() {
            return *ipAddress;
        }

        int getSocket() {
            return sock;
        }

        bool connect() {
            std::cout << "Fallback connect() Stage 1: " << strerror(errno) << std::endl;
            for (int i = 0; i < servers.size(); ++i) {
                int sock = createSocket();
                if (i == 0) {
                    std::cout << "Entered loop Stage 2: " << strerror(errno) << std::endl;
                }

                if (connectToServer(servers[i], sock)) {
                    this->sock = sock;
                    this->ipAddress = &servers[i];
                    std::cout << "Fallback connect() Stage 3: " << strerror(errno) << std::endl;
                    return true;
                } else {
                    std::cout << "Could not connect to server: " << servers[i];
                    std::cout << strerror(errno);
                }

                close(sock); // if connection not established, close the socket.
            }

            std::cout << "Fallback connect() Stage 5: " << strerror(errno) << std::endl;
            servers.clear();
            return false;
        }
};


void handleIncomingMessages(const int &sock) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        struct timeval timeout;
        timeout.tv_sec = HEARTBEAT_INTERVAL_SECONDS;
        timeout.tv_usec = 0;

        fd_set readSocks;
        fd_set *readSocksAddr = &readSocks;

        FD_ZERO(readSocksAddr);
        FD_SET(sock, readSocksAddr);

        int readySocks = select(sock + 1, readSocksAddr, nullptr, nullptr, &timeout);
        if (readySocks < 0) {
            std::cerr << "Error during select(): " << strerror(errno) << ". Disconnecting..." << std::endl;
            break;
        }

        sendHeartbeat(sock);
        std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL_SECONDS));

        int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (!isServerResponding(sock, bytesReceived)) {
            break;
        }

        if (strncmp(HEARTBEAT_RESPONSE, buffer, sizeof(HEARTBEAT_RESPONSE)) != 0) {
            std::cout << buffer << std::endl;
        }
    }

    std::cout << "Stage 1: " << strerror(errno) << std::endl;
    close(sock); 

    std::cout << "Stage 2: " << strerror(errno) << std::endl;
    if (servers.empty()) {
        return;
    }

    std::cout << "Stage 3: " << strerror(errno) << std::endl;

    FallbackServer fallbackServer;
    if (fallbackServer.connect()) {
        std::thread(handleIncomingMessages, fallbackServer.getSocket());
        std::cout << "Connected to server: " << fallbackServer.getIpAddress() << std::endl;
        std::cout << "Stage 4: " << strerror(errno) << std::endl;
    } else {
        std::cerr << "Could not connect to fallback servers." << std::endl;
        std::cout << "Stage 5: " << strerror(errno) << std::endl;
    }

}

int main() {
    const int sock = createSocket();
    std::string ipAddress;

    std::cout << "Enter IP Addresses by order, the first server will be the first you will connect to." << std::endl;
    std::cout << "If an unexpected server crash occurs, the client will behave the next server as fallback server." << std::endl;
    std::cout << "Invoke /done when you want to terminate the list." << std::endl;

    do {
        int serverId = servers.size() + 1;
        std::cout << "Enter #" << serverId << " IP Address: " << std::endl;
        std::cin >> ipAddress;

        if (ipAddress == "/done") {
            break;
        }

        // sockaddr_in serverAddress;
        // if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr) == 1) {
        //     std::cerr << "Invalid IP Address: " << ipAddress << strerror(errno) << std::endl;
        //     continue;
        // }

        std::lock_guard<std::mutex> lock(serverMutex);
        servers.push_back(ipAddress);

        if (servers.size() == MAX_FALLBACK_SERVERS) {
            std::cout << "Maximum allowed fallback servers reached. Terminating process.." << std::endl;
            break;
        }
    } while (ipAddress != "/done");

    if (servers.empty()) {
        std::cerr << "No servers provided." << std::endl;
        return 1;
    }

    std::string ip = servers[0];
    if (!connectToServer(ip, sock)) {
        std::cout << "Could not connect to server: " << ip << std::endl;
        
        FallbackServer fallbackServer;
        if (!fallbackServer.connect()) {
            std::cout << "Could not connect to fallback servers." << std::endl;
            return 1;
        }

        std::cout << "Connected to server: " << fallbackServer.getIpAddress() << std::endl;
    }

    std::cout << "Enter username: ";
    std::string username;
    std::cin >> username;
    send(sock, username.c_str(), username.length(), 0); // Sending username to server

    std::thread(handleIncomingMessages, sock).detach();    

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);
        std::string msg = std::string(buffer);

        size_t msgLen = msg.length();
        if (msgLen == 0) {
            continue;
        }

        if (msgLen > BUFFER_SIZE) {
            std::cerr << "Your message is too long. The maximum allowed size is " << BUFFER_SIZE << " characters." << std::endl;
            break;
        }

        if (msg == EXIT_CMD) {
            std::cout << "Disconnected from server: " << servers[0] << std::endl;
            servers.clear();
            close(sock);
            break;
        }

        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(sock, buffer, BUFFER_SIZE, 0) == -1) {
            std::cerr << "Could not send message to server: " << servers[0] << std::endl;
            continue;
        }
    }

    return 0;
}
