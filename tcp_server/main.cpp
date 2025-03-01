
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

   #ifdef _WIN32
        #include <conio.h>

        std::cout << "Press any key to continue..." << std::endl;
        _getchar();
   #endif // _WIN32
   return 0;
}