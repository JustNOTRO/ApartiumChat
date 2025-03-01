#include "Server.h"

int main() {
   std::cout << "Enter IP Address: ";
   std::string ipAddress;
   std::cin >> ipAddress;

   if (ipAddress.starts_with("localhost")) { 
       ipAddress.replace(0, ipAddress.length(), LOCAL_HOST_ADDRESS);
   }

   std::string ip = NetworkUtils::getSelectedIpAddress(ipAddress);
   std::uint16_t port = NetworkUtils::getSelectedPort(ipAddress);

   Server server(ip, port);
   server.run();
   return 0;
}