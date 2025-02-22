#include <iostream>
#include "Server.h"
#include "utils/ServerUtils.h"
#include "ServerConstants.h"

class Server;
class ServerUtils;

int main() {
   std::cout << "Enter IP Address: ";
   std::string ipAddress;
   std::cin >> ipAddress;

   if (ipAddress == "localhost") {
       ipAddress = LOCAL_HOST_ADDRESS;
   }

   std::string ip = ServerUtils::getSelectedIpAddress(ipAddress);
   short port = ServerUtils::getSelectedPort(ipAddress);

   Server server(ip, port);
   server.run();
   return 0;
}