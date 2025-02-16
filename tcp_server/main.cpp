#include <iostream>
#include "Server.h"
#include "Client.h"

class Server;
class Client;

int main() {
    std::cout << "Enter server port: ";
    short port;
    std::cin >> port;

    Server server(port);
    server.run();
    return 0;
}