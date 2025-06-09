#ifndef SERVER_H
#define SERVER_H

#include "router.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    int fd;
    int port;
    Router router;
} HttpServer;

ParseResult _parse_request_message(const char* message);
HttpMethod _parse_method(const char* method);
size_t _build_response(int status_code, const char* body, char** respones);

HttpServer* create_server(int port);
ServerStatus run_server(const HttpServer* server);
ServerStatus close_server(HttpServer* server);

#endif // SERVER_H
