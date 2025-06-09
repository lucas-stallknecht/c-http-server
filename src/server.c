#include "server.h"
#include "animal_controller.h"
#include "router.h"
#include "types.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_HANDLED_METHODS 2
#define MAX_ROUTES 8
#define LISTEN_BACKLOG 10

#define LOG_SERVER_ERR(msg) perror(msg);

static const char* const methods[NUM_HANDLED_METHODS] = {
    [GET] = "GET",
    [POST] = "POST",
};

HttpMethod _parse_method(const char* method) {
    for (int i = 0; i < NUM_HANDLED_METHODS; i++) {
        if (strcmp(method, methods[i]) == 0) {
            return i;
        }
    }
    return UNKNOWN_METHOD;
}

ParseResult _parse_request_message(const char* message) {
    ParseResult res = {.requested_route.method = UNKNOWN_METHOD};

    char* start_line = strsep((char**)&message, "\n");
    char method[16], path[256], http_version[16];

    if (sscanf(start_line, "%15s %255s %15s", method, path, http_version) != 3) {
        res.status = PARSE_FAILED_FORMAT;
        return res;
    }

    if (strncmp(http_version, "HTTP/", 5) != 0) {
        res.status = PARSE_FAILED_FORMAT;
        return res;
    }

    HttpMethod method_value = _parse_method(method);
    if (method_value == -1) {
        res.status = PARSE_FAILED_METHOD;
        return res;
    }

    // All routes must start with /
    if (path[0] != '/') {
        res.status = PARSE_FAILED_PATH;
        return res;
    }

    size_t path_length = strlen(path);

    // run_server is reponsible of freeing this
    res.requested_route.path = malloc(path_length + 1);
    if (!res.requested_route.path) {
        res.status = PARSE_FAILED_SERVER;
        return res;
    }

    memcpy(res.requested_route.path, path, path_length);
    res.requested_route.path[path_length] = '\0';
    res.requested_route.path_length = path_length;
    res.requested_route.method = method_value;
    res.status = PARSE_OK;

    return res;
}

size_t _build_response(int status_code, const char* body, char** response) {
    const char* status_text;
    switch (status_code) {
    case 200:
        status_text = "200 OK";
        break;
    case 400:
        status_text = "400 Bad Request";
        break;
    case 404:
        status_text = "404 Not Found";
        break;
    default:
        status_text = "500 Internal Server Error";
        break;
    }

    // Only return html for well formatted requests
    const char* content_type = (status_code == 400 || status_code == 500) ? "plain" : "html";
    size_t body_length = body ? strlen(body) : 0;

    char header[512];
    int header_length = snprintf(header, sizeof(header),
                                 "HTTP/1.1 %s\r\n"
                                 "Content-Type: text/%s\r\n"
                                 "Content-Length: %zu\r\n\r\n",
                                 status_text, content_type, body_length);

    if (header_length < 0) {
        *response = NULL;
        return 0;
    }

    size_t response_length = header_length + body_length;
    // run_server is responsible of freeing it
    *response = malloc(response_length);
    if (!*response) {
        return 0;
    }

    memcpy(*response, header, header_length);
    if (body_length > 0) {
        memcpy(*response + header_length, body, body_length);
    }

    return response_length;
}

HttpServer* create_server(int port) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1) {
        LOG_SERVER_ERR("Failed to create socket\n");
        return NULL;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = INADDR_ANY},
        .sin_port = htons(port),
    };

    if (bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        LOG_SERVER_ERR("Failed to bind socket\n");
        close(serverfd);
        return NULL;
    }

    // the caller is responsible of freeing it
    HttpServer* server = malloc(sizeof(HttpServer));
    if (!server) {
        close(serverfd);
        return NULL;
    }

    server->port = port;
    server->fd = serverfd;
    server->router = create_router(MAX_ROUTES);
    return server;
}

ServerStatus run_server(const HttpServer* server) {
    if (listen(server->fd, LISTEN_BACKLOG) == -1) {
        LOG_SERVER_ERR("Failed to listen\n");
        return SERVER_ERR_LISTEN;
    }

    fprintf(stderr, "Listening on port %d...\n", server->port);

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1) {
        // Accept client connections
        int clientfd = accept(server->fd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (clientfd == -1) {
            fprintf(stderr, "Could not accept connection from %s:%d\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            break;
        }

        char read_buff[4096];
        int recv_status = recv(clientfd, read_buff, sizeof(read_buff), 0);
        if (recv_status == -1) {
            fprintf(stderr, "Could not read from %s\n", inet_ntoa(client_addr.sin_addr));
            close(clientfd);
            continue;
        }

        // Parse the request, match the route and build the response
        char* response = NULL;
        size_t response_length = 0;
        ParseResult parse_result = _parse_request_message(read_buff);

        if (parse_result.status != PARSE_OK) {
            response_length = _build_response(400, NULL, &response);
        } else {
            HttpRoute* route = &parse_result.requested_route;
            ControllerFunc controller_func;
            RouteMatchStatus match = router_get_function(&server->router, route, &controller_func);

            char* response_body = NULL;
            size_t response_body_length = 0;

            if (match == MATCH_OK && controller_func) {
                controller_func(&response_body, &response_body_length);
                response_length = _build_response(200, response_body, &response);
            } else {
                not_found_func(&response_body, &response_body_length);
                response_length = _build_response(404, response_body, &response);
            }

            free(response_body);
        }

        if (send(clientfd, response, response_length, 0) == -1) {
            fprintf(stderr, "Could not send response to %s\n", inet_ntoa(client_addr.sin_addr));
        }

        free(response);
        free(parse_result.requested_route.path);
        close(clientfd);
    }

    return SERVER_OK;
}

ServerStatus close_server(HttpServer* server) {
    fprintf(stderr, "\nClosing server (port %d)\n", server->port);
    if (close(server->fd) == -1) {
        LOG_SERVER_ERR("Failed to close server socket");
        return SERVER_ERR_CLOSE;
    }

    return SERVER_OK;
}
