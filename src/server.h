#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

#define LISTEN_BACKLOG 10

#define LOG_SERVER_ERR(msg) perror(msg);

typedef enum {
    SERVER_OK = 0,
    SERVER_ERR_LISTEN,
    SERVER_ERR_ACCEPT,
    SERVER_ERR_CLOSE,
    SERVER_ERR_UNKNOWN
} server_error_t;


typedef struct {
    int fd;
    int port;
} http_server_t;

http_server_t* create_server(int port);
server_error_t run_server(http_server_t* server);
server_error_t close_server(http_server_t* server);

#endif // SERVER_H
