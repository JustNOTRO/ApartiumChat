#pragma once
#ifndef SERVERCONSTANTS_H
#define SERVERCONSTANTS_H

#define BUFFER_SIZE 1500
#define DEFAULT_PORT 8080
#define HEARTBEAT_INTERVAL_SECONDS 5
#define HEARTBEAT_REQUEST "/ping"
#define HEARTBEAT_RESPONSE "/pong"
#define EXIT_CMD "/exit"
#define LOCAL_HOST_ADDRESS "127.0.0.1:8080"
#define MAX_FALLBACK_SERVERS 10 // change this asap to json, this is only tmp

#endif // SERVERCONSTANTS_H