#include "server.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

http_server_t* create_server(int port) {
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

    http_server_t* server = malloc(sizeof(http_server_t));
    server->port = port;
    server->fd = serverfd;

    return server;

}

server_error_t run_server(http_server_t* server) {
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

        // printf("%s", read_buff);

        const char response[] = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                                "yo";

        send(clientfd, &response, sizeof(response), 0);

        if (close(clientfd) == -1) {
            printf("Could not close connection from %s\n",
                   inet_ntoa(client_addr.sin_addr));
            continue;
        }
    }
    return SERVER_OK;
}

server_error_t close_server(http_server_t* server) {
    printf("Closing server (port %d)\n", server->port);
    if (close(server->fd) == -1) {
        LOG_SERVER_ERR("Failed to close server socket");
        return SERVER_ERR_CLOSE;
    }

    return SERVER_OK;
}


