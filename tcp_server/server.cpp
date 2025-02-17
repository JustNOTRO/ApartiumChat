#include <iostream>
#include "Server.h"
#include "utils/ServerUtils.h"

class Server;
class ServerUtils;

int main() {
    std::cout << "Enter Server Address: ";
    std::string serverAddr;
    std::cin >> serverAddr;

    std::string ip = ServerUtils::getSelectedIpAddress(serverAddr);
    short port = ServerUtils::getSelectedPort(serverAddr);

    Server server(ip, port);
    server.run();
    return 0;
}