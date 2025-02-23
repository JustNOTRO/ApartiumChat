#pragma once
#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <iostream>

class ServerUtils {

    public:
        static std::string getSelectedIpAddress(std::string serverAddr);
        
        static short getSelectedPort(std::string serverAddr);
};

#endif // SERVERUTILS_H