#include <iostream>
#include "ServerUtils.h"
#include "ServerConstants.h"


std::string ServerUtils::getSelectedIpAddress(std::string providedAddress) {
    size_t colonPos = providedAddress.find(':');
    return providedAddress.substr(0, colonPos);
}

short ServerUtils::getSelectedPort(std::string ipAddress) {
    size_t colonPos = ipAddress.find(':');
    if (colonPos == std::string::npos) {
        return DEFAULT_PORT;
    }

    // Parse the port as short since we don't need that many bytes.
    short port = static_cast<short>(std::stoi(ipAddress.substr(colonPos + 1)));
    return port;
}