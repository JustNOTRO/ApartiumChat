#include <string.h>
#include <memory>
#include "Logger.h"
#include "SocketAdapter.h"

void Logger::logLastError(std::string message) {
    std::cerr << message << " " << getLastError() << " ." << std::endl;
}

#ifdef _WIN32
std::string Logger::getLastWindowsError() {
    int errorCode = WSAGetLastError();
    char* msgBuffer = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuffer, 0, nullptr
    );

    std::unique_ptr<char, decltype(&LocalFree)> msg(msgBuffer, LocalFree);
    if (!msg) {
        logLastError("Could not retrieve error message");
        exit(EXIT_FAILURE);
    }

    return std::string(msg.get());
}
#endif // _WIN32


std::string Logger::getLastError() {
    #ifdef _WIN32
        return getWindowsError();
    #else
        return strerror(errno);
    #endif // _WIN32
}