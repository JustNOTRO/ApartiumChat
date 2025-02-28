#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <optional>
#include <string>

#include "NetworkUtils.h"
#include "ServerConstants.h"
#include "Logger.h"

std::mutex serverMutex;
std::atomic<bool> disconnected(false);

Socket sock;

/**
 * @brief Updates the socket safe.
 * @param newSock the new socket assigned
 */
void updateSocket(Socket newSock) {
    std::lock_guard<std::mutex> lock(serverMutex);
    sock = newSock;
}

/**
 * @brief Gets the current socket.
 */
Socket getCurrentSocket() {
    std::lock_guard<std::mutex> lock(serverMutex);
    return sock;
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
     * @brief Constructs new fallback server.
     * @param ipAddress the server address
     * @param newSock the server socket
     */
    FallbackServer(const std::string &ipAddress, Socket newSock)
        : ipAddress(ipAddress), newSock(newSock) {}

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
};

/**
 * @brief Connects to the server on the provided IP Address.
 * @param ipAddress the IP address to connect to
 * @param sock socket 
 * @return true if connection established, false otherwise
 */
bool connectToServer(const std::string &ipAddress, Socket sock) {
    std::string ip = NetworkUtils::getSelectedIpAddress(ipAddress);
    std::uint16_t port = NetworkUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) == -1) {
        Logger::logLastError("Invalid IP Address");
        return false;
    }
    
    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

std::vector<FallbackServer> servers;

/**
 * @brief Gets the next available server in the provided fallback servers.
 * @return An optional reference wrapper to a FallbackServer if one is found; std::nullopt otherwise.
 */
std::optional<std::reference_wrapper<FallbackServer>> getNextAvailableServer() {
    std::lock_guard<std::mutex> lock(serverMutex);
    
    for (auto &server : servers) {
        Socket fallbackSock = server.getSocket();

        if (connectToServer(server.getIpAddress(), fallbackSock)) {
            return server;
        }

    }

    return std::nullopt;
}

/**
 * @brief Sends heartbeat to the server to indicate that the client is alive.
 * @param sock the server socket to send the heartbeat to it
 */
void sendHeartbeat(Socket sock) {
    if (disconnected.load()) {
        return;
    }

    if (send(sock, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) == -1) {
        Logger::logLastError("Could not send heartbeat");
        return;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @brief Determines if the server is responding or not.
 * @param buffer the message buffer
 * @return true if responding, false otherwise
 */
bool isServerResponding(char buffer[]) {
    Socket currentSock = getCurrentSocket();
    struct timeval timeout { HEARTBEAT_INTERVAL_SECONDS, 0 };
    fd_set readSocks;
    FD_ZERO(&readSocks);
    FD_SET(currentSock, &readSocks);

    int readySocks = select(currentSock + 1, &readSocks, nullptr, nullptr, &timeout);
    if (readySocks < 0) {
        Logger::logLastError("Error during select()");
        std::cerr << "Disconnecting.";
        return false;
    }
    
    if (readySocks == 0) {
        sendHeartbeat(currentSock);
    }

    int bytesReceived = recv(currentSock, buffer, BUFFER_SIZE, 0);
    return bytesReceived > 0;
}

/**
 * @brief Indicates that the received message is a heartbeat response.
 * @param buffer the message sent
 * @return true if heartbeat response, false otherwise
 */
bool isHeartbeatResponse(char buffer[]) {
    return strncmp(buffer, HEARTBEAT_RESPONSE, strlen(HEARTBEAT_RESPONSE)) == 0;
}

/**
 * @brief Handles the incoming messages of other clients.
 */
void handleIncomingMessages(const std::string &username) {
    char buffer[BUFFER_SIZE];

    while (!disconnected.load()) {
        memset(buffer, 0, BUFFER_SIZE);

        if (isServerResponding(buffer)) {
            if (!isHeartbeatResponse(buffer)) {
                std::cout << buffer << std::endl;
            }

            if (strcmp(buffer, USERNAME_TAKEN_MSG) == 0) {
                disconnected.store(true);
                break;
            }
            
            continue;
        }

        std::cerr << "Server is not responding, disconnecting.." << std::endl;

        // We terminate if only one server is provided.
        if (servers.size() == 1) {
            break;
        }
        
        auto serverOpt = getNextAvailableServer();
        if (!serverOpt.has_value()) {
            Logger::logLastError("Could not connect to fallback servers");
            break;
        }

        updateSocket(serverOpt.value().get().getSocket());
        send(getCurrentSocket(), username.c_str(), username.length(), 0);
        std::cout << "Connected to fallback server: " << serverOpt.value().get().getIpAddress() << std::endl;
    }
}

/**
 * @brief Handles client input and sends messages.
 */
void handleClientInput() {
    char buffer[BUFFER_SIZE];

    while (!disconnected.load()) {
        memset(buffer, 0, BUFFER_SIZE);
        std::cin.getline(buffer, BUFFER_SIZE);
        std::string msg(buffer);
        
        if (msg.empty()) {
            continue;
        }
        
        if (msg.length() >= BUFFER_SIZE) {
            std::cerr << "Message too long." << std::endl;
            continue;
        }
        
        if (msg == EXIT_CMD) {
            disconnected.store(true);
            NetworkUtils::shutdownSocket(getCurrentSocket());
            break;
        }
        
        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(getCurrentSocket(), msg.c_str(), msg.length(), 0) == -1) {
            Logger::logLastError("Could not send message");
        }
    }
}

int main() {
    updateSocket(NetworkUtils::createSocket());

    std::cout << "Enter IP Addresses: (type /done to finish):" << std::endl;
    std::string ipAddress;

    do {
        std::lock_guard<std::mutex> lock(serverMutex);
        
        std::cin >> ipAddress;
        if (ipAddress == "/done") {
            break;
        }

        Socket fallbackSock = NetworkUtils::createSocket();
        FallbackServer fallbackServer{ ipAddress, fallbackSock };
        servers.push_back(fallbackServer);
    } while (ipAddress != "/done");
    
    if (servers.empty()) {
        std::cerr << "No servers provided." << std::endl;
        return 1;
    }
    
    if (!connectToServer(servers[0].getIpAddress(), getCurrentSocket())) {
        Logger::logLastError("Could not connect to server " + servers[0].getIpAddress());

        // if only 1 server provided
        if (servers.size() == 1) {
            return 0;
        }

        auto serverOpt = getNextAvailableServer();
        if (!serverOpt.has_value()) {
            Logger::logLastError("Could not connect to fallback servers");
            return 1;
        }

        updateSocket(serverOpt.value().get().getSocket());
        std::cout << "Connected to fallback server: " << serverOpt.value().get().getIpAddress() << std::endl;
    }
    
    std::string username;
    while (username.empty()) {
        std::cout << "Enter username: ";
        std::cin >> username;
    }
    
    send(getCurrentSocket(), username.c_str(), username.length(), 0);
    std::thread recvThread(handleIncomingMessages, username);
    handleClientInput();
    recvThread.join();

    servers.clear();
    NetworkUtils::closeSocket(getCurrentSocket());

    #ifdef _WIN32
        if (disconnected.load()) {
            WSACleanup();
        }
    #endif // _WIN32

    return 0;
}
