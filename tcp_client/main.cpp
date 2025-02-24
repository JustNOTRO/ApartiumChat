#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

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

#ifdef _WIN32
    using Socket = SOCKET;
#else
    using Socket = int;
#endif

std::vector<std::string> servers;
std::mutex serverMutex;
std::atomic<Socket> sock;

#ifdef _WIN32
std::string getWindowsError() {
    int errorCode = WSAGetLastError();
    char* msgBuffer = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuffer, 0, nullptr
    );

    std::unique_ptr<char, decltype(&LocalFree)> msg(msgBuffer, LocalFree);
    if (!msg) {
        logError("Could not retrieve error message");
        exit(EXIT_FAILURE);
    }

    return std::string(msg.get());
#endif

void logError(std::string message) {
    #ifdef _WIN32
        std::cerr << message << ": " << getWindowsError() << "." << std::endl;
    #else
        std::cerr << message << ": " << strerror(errno) << std::endl;
    #endif
}

void closeSocket(Socket sock) {
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
}

Socket createSocket() {
    int newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (newSock < 0) {
        logError("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    return newSock;
}

bool sendHeartbeat(Socket sock) {
    if (send(sock, HEARTBEAT_REQUEST, sizeof(HEARTBEAT_REQUEST), 0) == -1) {
        logError("Could not send heartbeat");
        return false;
    }

    return true; 
}

bool isServerResponding(int &bytesReceived) {
    if (bytesReceived > 0) {
        return true;
    }

    if (bytesReceived < 0) {
        logError("Error receiving data");
    }

    std::cerr << "Server is not responding.. disconnecting.." << std::endl;
    return false;
}

bool connectToServer(const std::string &ipAddress, Socket sock) {
    std::string ip = ServerUtils::getSelectedIpAddress(ipAddress);
    short port = ServerUtils::getSelectedPort(ipAddress);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        logError("Invalid IP Address");
        return false;
    }

    return connect(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0;
}

struct FallbackServer {

private:
    std::string ipAddress;
    Socket newSock;

public:
    std::string getIpAddress() {
        return ipAddress;
    }

    Socket getSocket() {
        return newSock;
    }

    bool connect() {
        if (servers.empty()) {
            return false;
        }

        for (const auto &server : servers) {
            Socket sock = createSocket();
            if (connectToServer(server, sock)) {
                newSock = sock;
                ipAddress = server;
                return true;
            }

            closeSocket(sock);
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
            logError("Error while waiting for ready sockets");
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
            logError("Could not connect to fallback servers");
            break;
        }

        sock.store(fallbackServer.getSocket());
        send(sock.load(), username.c_str(), username.length(), 0);
        std::cout << "Connected to fallback server: " << fallbackServer.getIpAddress() << std::endl;
    }

    closeSocket(sock.load());
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
            closeSocket(sock);
            break;
        }
        
        if (msg == HEARTBEAT_REQUEST) {
            continue;
        }

        if (send(sock.load(), msg.c_str(), msg.length(), 0) == -1) {
            logError("Could not send message");
        }
    }
}

int main() {
    #ifdef _WIN32
        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            logError("WSAStartup failed");
            return 1;
        }
    
    #endif
    
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
            closeSocket(sock);
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
    closeSocket(sock);
    return 0;
}
