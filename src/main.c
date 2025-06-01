#include "server.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static http_server_t* global_server = NULL;

void cleanup_server() {
    if (global_server != NULL) {
        close_server(global_server);
        free(global_server);
        global_server = NULL;
    }
    exit(0);
}

void handle_sigint(int sig) {
    cleanup_server();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("You must only specify a port for the server");
        return 1;
    }

    // Parse port as int from argv
    const char* first_param = argv[1];
    int port = atoi(first_param);
    if (port == 0) {
        printf("Failed to read port number");
        return 1;
    }

    global_server = create_server(port);
    if (!global_server)
        return 1;

    signal(SIGINT, handle_sigint);
    run_server(global_server);
    cleanup_server();

    return 0;
}
