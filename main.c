#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define LISTEN_BACKLOG 10

#define HANDLE_ERROR(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int main() {
    // Create the socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1)
        HANDLE_ERROR("Failed to create socket");

    // Bind an address to the socket (aka assigning a name to the socket)
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = INADDR_ANY},
        .sin_port = htons(SERVER_PORT),
    };

    if (bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        HANDLE_ERROR("Failed to bind socket");

    if (listen(serverfd, LISTEN_BACKLOG) == -1)
        HANDLE_ERROR("Failed to listen");

    printf("Listening on port %d...\n", SERVER_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_addn_size = sizeof(client_addr);
    while (1) {
        // Now, we can accept connections one at the time
        int clientfd = accept(serverfd,
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

        printf("%s", read_buff);

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

    if (close(serverfd) == -1)
        HANDLE_ERROR("Failed to close server socket");

    return 0;
}
