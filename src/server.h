#ifndef SERVER_H
#define SERVER_H

#include "types.h"
#include "router.h"
#include <stdbool.h>
#include <stdio.h>

#define LISTEN_BACKLOG 10

#define LOG_SERVER_ERR(msg) perror(msg);

typedef enum {
    SERVER_OK = 0,
    SERVER_ERR_LISTEN,
    SERVER_ERR_ACCEPT,
    SERVER_ERR_CLOSE,
    SERVER_ERR_UNKNOWN
} ServerStatus;

typedef struct {
    int fd;
    int port;
    Router router;
} HttpServer;

HttpServer* create_server(int port);
ServerStatus run_server(const HttpServer* server);
ServerStatus close_server(HttpServer* server);

ParseResult parse_request_message(const char* message);
HttpMethod parse_method(const char* method);

#endif // SERVER_H
