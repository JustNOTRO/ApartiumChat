#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

/**
 * @brief Logger is a utility class that his main purpose is logging error messages indicated by different operating systems
 */
class Logger {

    public:

        /**
         * @brief Logs last error indicated by the system, with additional message.
         * @param message
         */
        static void logLastError(std::string message);
    private:
        Logger();

        /**
         * @brief Gets the last error indicated by windows system.
         */
        #ifdef _WIN32
            static std::string getLastWindowsError();
        #endif // _WIN32

        /**
         * @brief Gets the last error indicated by the system.
         */
        static std::string getLastError();

};

#endif // LOGGER_H