#pragma once
#ifndef SERVERCONSTANTS_H
#define SERVERCONSTANTS_H

#define BUFFER_SIZE 1500
#define DEFAULT_PORT 8080
#define HEARTBEAT_INTERVAL_SECONDS 5
#define HEARTBEAT_REQUEST "/ping"
#define HEARTBEAT_RESPONSE "/pong"
#define EXIT_CMD "/exit"
#define LOCAL_HOST_ADDRESS "127.0.0.1"
#define USERNAME_TAKEN_MSG "Username already taken, Please try other name."

#endif // SERVERCONSTANTS_H