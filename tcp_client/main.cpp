#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

#include "NetworkUtils.h"
#include "ServerConstants.h"
#include "Logger.h"

std::vector<std::string> servers;
std::mutex serverMutex;
std::atomic<Socket> sock;
std::atomic<bool> disconnected(false);

/**
 * @brief Sends heartbeat to the server for indicating if the server has crashed or not.
 * @param sock the socket to send the message to.
 */
bool sendHeartbeat(Socket sock) {
    if (send(sock, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) == -1) {
        Logger::logLastError("Could not send heartbeat");
        return false;
    }

    return true; 
}

/**
 * @brief Determnines if the server is responding or not.
 * @param bytesReceived the amount of bytes received from recv()
 * @return true if responding, false otherwise
 */
bool isServerResponding(int &bytesReceived) {
    if (bytesReceived > 0) {
        return true;
    }

    if (bytesReceived < 0) {
        Logger::logLastError("Error receiving data");
    }

    std::cerr << "Server is not responding.. disconnecting.." << std::endl;
    return false;
}

/**
 * @brief Connects to the server on the provided IP Address.
 * @param ipAddress the IP address to connect to
 * @param sock socket 
 * @return true if connection established, false otherwise
 */
bool connectToServer(const std::string &ipAddress, Socket sock) {
    std::string ip = NetworkUtils::getSelectedIpAddress(ipAddress);
    short port = NetworkUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        Logger::logLastError("Invalid IP Address");
        return false;
    }

    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

/**
 * @brief Fallback server is a struct representation of the fallback server of which the client is connected to.
 * @see Failover Protocol https://en.wikipedia.org/wiki/Failover
 */
struct FallbackServer {

private:
    std::string ipAddress;
    Socket newSock;

public:
    /**
     * @brief Gets the IP Address of the fallback server which the client is connected to.
     * @return the IP Address of the fallback server
     */
    std::string getIpAddress() {
        return ipAddress;
    }

    /**
     * @brief Gets the fallback socket
     * @return fallback socket
     */
    Socket getSocket() {
        return newSock;
    }

    /**
     * @brief Attempts to connect to any available fallback server in provided servers.
     * @return true if connection established, false otherwise
     */
    bool connect() {
        std::lock_guard<std::mutex> lock(serverMutex);

        if (servers.empty()) {
            return false;
        }

        for (const auto &server : servers) {
            Socket sock = NetworkUtils::createSocket();
            if (connectToServer(server, sock)) {
                newSock = sock;
                ipAddress = server;
                return true;
            }

            NetworkUtils::closeSocket(sock);
        }

        return false;
    }
};

/**
 * @brief Handles the incoming messages of other clients.
 */
void handleIncomingMessages(const std::string &username) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        struct timeval timeout = {HEARTBEAT_INTERVAL_SECONDS, 0};
        fd_set readSocks;
        FD_ZERO(&readSocks);
        FD_SET(sock.load(), &readSocks);

        // no need to continue if client disconnected.
        if (disconnected.load()) { 
            break;
        }

        int readySocks = select(sock.load() + 1, &readSocks, nullptr, nullptr, &timeout);
        if (readySocks < 0) {
            Logger::logLastError("Error while waiting for ready sockets");
            break;
        }

        if (readySocks == 0) {
            sendHeartbeat(sock.load());
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        int bytesReceived = recv(sock.load(), buffer, BUFFER_SIZE, 0);
        if (isServerResponding(bytesReceived)) {
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
            Logger::logLastError("Could not connect to fallback servers");
            break;
        }

        sock.store(fallbackServer.getSocket());
        send(sock.load(), username.c_str(), username.length(), 0);
        std::cout << "Connected to fallback server: " << fallbackServer.getIpAddress() << std::endl;
    }

    memset(buffer, 0, BUFFER_SIZE);
    NetworkUtils::closeSocket(sock.load());
}

/**
 * @brief Handles the client input and sends it to all connected clients.
 */
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
            memset(buffer, 0, BUFFER_SIZE);
            disconnected.store(true);
            break;
        }
        
        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(sock.load(), msg.c_str(), msg.length(), 0) == -1) {
            Logger::logLastError("Could not send message");
        }
    }

    NetworkUtils::closeSocket(sock);
}

int main() {
    sock.store(NetworkUtils::createSocket());

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

        if (servers.size() == 1) {
            NetworkUtils::closeSocket(sock);
            return 0;
        }

        FallbackServer fallbackServer;
        if (!fallbackServer.connect()) {
            std::cerr << "Could not connect to fallback servers." << std::endl;
            NetworkUtils::closeSocket(sock);
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
