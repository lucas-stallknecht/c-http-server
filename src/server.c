#include "server.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static const char* const methods[NUM_HANDLED_METHODS] = {
    [GET] = "GET",
    [POST] = "POST",
};

HttpServer* create_server(int port) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1) {
        LOG_SERVER_ERR("Failed to create socket");
        return NULL;
    }

    // Bind an address to the socket (aka assigning a name to the socket)
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = INADDR_ANY},
        .sin_port = htons(port),
    };

    if (bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        LOG_SERVER_ERR("Failed to bind socket");
        return NULL;
    }

    HttpServer* server = malloc(sizeof(HttpServer));
    server->port = port;
    server->fd = serverfd;

    return server;
}

ServerStatus run_server(const HttpServer* server) {
    if (listen(server->fd, LISTEN_BACKLOG) == -1) {
        LOG_SERVER_ERR("Failed to listen");
        return SERVER_ERR_LISTEN;
    }

    printf("Listening on port %d...\n", server->port);

    struct sockaddr_in client_addr;
    socklen_t client_addn_size = sizeof(client_addr);
    while (1) {
        // Now, we can accept connections one at the time
        int clientfd = accept(server->fd,
                              (struct sockaddr*)&client_addr,
                              &client_addn_size);
        if (clientfd == -1) {
            printf("Could not accept connection from %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            break;
        }

        char read_buff[4096];
        int receive_status = recv(clientfd, &read_buff, sizeof(read_buff), 0);

        if (receive_status == -1) {
            printf("Could not read from %s\n",
                   inet_ntoa(client_addr.sin_addr));
            continue;
        }

        // -- Request parsing
        ParseResult result = parse_request_message(read_buff);

       // -- Building dummy response with parse infos
        char response[1024];

        const char* response_header = "HTTP/1.1 200 OK\r\n"
                                      "Content-Type: text/html; charset=UTF-8\r\n\r\n";

        char response_body[100];
        sprintf(response_body, "Parse status: %d ,Method: %d, Path: %s \r\n",
                result.status,
                result.requested_route.method,
                result.requested_route.path);

        int response_length = snprintf(response,
                                       sizeof(response),
                                       "%s%s",
                                       response_header, response_body);

        send(clientfd, &response, response_length, 0);

        // -- Cleanup parse result
        free(result.requested_route.path);

        if (close(clientfd) == -1) {
            printf("Could not close connection from %s\n",
                   inet_ntoa(client_addr.sin_addr));
            continue;
        }
    }
    return SERVER_OK;
}

ServerStatus close_server(HttpServer* server) {
    printf("\nClosing server (port %d)\n", server->port);
    if (close(server->fd) == -1) {
        LOG_SERVER_ERR("Failed to close server socket");
        return SERVER_ERR_CLOSE;
    }

    return SERVER_OK;
}

HttpMethod parse_method(const char* method) {
    for (int i = 0; i < NUM_HANDLED_METHODS; i++) {
        if (strcmp(method, methods[i]) == 0) {
            return i;
        }
    }
    return UNKNOWN_METHOD;
}

ParseResult parse_request_message(const char* message) {
    ParseResult res;
    res.requested_route.method = UNKNOWN_METHOD;

    // Only checks start lines for now
    char* start_line = strsep((char**)&message, "\n");

    // Matches line layout
    char method[16], path[256], http_version[16];
    int matched = sscanf(start_line, "%15s %255s %15s", method, path, http_version);
    if (matched != 3) {
        res.status = PARSE_FAILED_FORMAT;
        return res;
    }

    // Validate HTTP version starts with "HTTP/"
    if (strncmp(http_version, "HTTP/", 5) != 0) {
        res.status = PARSE_FAILED_FORMAT;
        return res;
    }

    HttpMethod method_value = parse_method(method);
    if (method_value == -1) {
        res.status = PARSE_FAILED_METHOD;
        return res;
    }

    // All routes should start with /
    if (path[0] != '/') {
        res.status = PARSE_FAILED_PATH;
        return res;
    }

    size_t path_length = strlen(path);

    res.status = PARSE_OK;
    res.requested_route = (HttpRoute){
        .path = malloc(path_length * sizeof(char)),
        .path_length = path_length,
        .method = method_value};
    memcpy(res.requested_route.path, path, path_length);

    return res;
}
