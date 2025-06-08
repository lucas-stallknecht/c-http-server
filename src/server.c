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
#define MAX_ROUTES 64 // Power of two

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

    HttpMethod method_value = _parse_method(method);
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
        .path = malloc(path_length * sizeof(char) + 1),
        .path_length = path_length,
        .method = method_value};

    if (res.requested_route.path == NULL) {
        res.status = PARSE_FAILED_SERVER;
        return res;
    }

    memcpy(res.requested_route.path, path, path_length);
    res.requested_route.path[path_length] = '\0';

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

    char header_format[] = "HTTP/1.1 %s\r\n"
                           "Content-Type: text/%s\r\n"
                           "Content-Length: %zu\r\n"
                           "\r\n";

    char* content_type = status_code == 400 ? "plain" : "html";
    size_t body_len = body ? strlen(body) : 0;

    // Estimate header size
    char header[512];
    int header_len = snprintf(header, sizeof(header), header_format, status_text, content_type, body_len);

    if (header_len < 0) {
        printf("Header formatting error\n");
        *response = NULL;
        return 0;
    }

    size_t response_len = header_len + body_len;

    *response = malloc(response_len);
    if (!*response) {
        perror("malloc failed");
        return 0;
    }

    memcpy(*response, header, header_len);
    if (body_len > 0) {
        memcpy(*response + header_len, body, body_len);
    }

    return response_len;
}

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
    server->router = create_router(MAX_ROUTES);

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
        // -- Accept client connections
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
            close(clientfd);
            continue;
        }

        char* response = NULL;
        size_t response_length = 0;

        // -- Request parsing
        ParseResult parse_result = _parse_request_message(read_buff);

        // -- Build the response
        if (parse_result.status != PARSE_OK) {
            response_length = _build_response(400, NULL, &response);
        } else {
            HttpRoute requested_route = {
                .method = parse_result.requested_route.method,
                .path = parse_result.requested_route.path,
                .path_length = parse_result.requested_route.path_length};

            ControllerFunc controller_func;
            RouteMatchStatus match = router_get_function(&server->router, &requested_route, &controller_func);

            char* response_body = NULL;
            size_t response_body_size = 0;
            if (match == MATCH_OK && controller_func != NULL) {
                controller_func(&response_body, &response_body_size);

                if (response_body != NULL)
                    response_length = _build_response(200, response_body, &response);

            } else {
                not_found_func(&response_body, &response_body_size);

                if (response_body != NULL)
                    response_length = _build_response(404, response_body, &response);
            }

            free(response_body);
        }

        // -- Send response
        int sent = send(clientfd, response, response_length, 0);
        if (sent == -1) {
            printf("Could not send response to %s\n", inet_ntoa(client_addr.sin_addr));
        }

        // -- Cleanup resources
        free(response);
        free(parse_result.requested_route.path);

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
