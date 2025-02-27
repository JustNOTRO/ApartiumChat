#pragma once
#ifndef NETWORKUTILS_H 
#define NETWORKUTILS_H

#include <cstdint>
#include "SocketAdapter.h"

/**
 * @brief NetworkUtils is a utility class that contains common methods upon server-client side applications
 */
class NetworkUtils {

    public:
        /**
         * @brief Gets the IP address part out of the full IP address, for example 127.0.0.1 out of 127.0.0.1:8080.
         * @param ipAddress the full IP address
         * @return the IP address out of the Full address
         */
        static std::string getSelectedIpAddress(std::string ipAddress);
        
        /**
         * @brief Gets the Port part out of the full IP address, for example 8080 out of 127.0.0.1:8080.
         * @param ipAddress the full IP address
         * @return the port out of the full address
         */
        static std::uint16_t getSelectedPort(std::string ipAddress);

        /**
         * @brief Creates a socket depending on the operating system, winsock for windows, unix-sockets for unix-like operating systems.
         */
        static Socket createSocket();

        /**
         * @brief Close the provided socket.
         * @param sock the socket to close
         */
        static void closeSocket(Socket sock);

        /**
         * @brief Shutdown the sock entirely.
         * @param sock the sock to shutdown
         */
        static void shutdownSocket(Socket sock);

    private:
        NetworkUtils();
};

#endif // NETWORKUTILS_H