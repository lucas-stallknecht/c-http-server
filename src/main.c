#include "router.h"
#include "server.h"
#include "animal_controller.h"
#include "types.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static HttpServer* global_server = NULL;

void cleanup_server() {
    if (global_server != NULL) {
        close_server(global_server);
        destroy_router(&global_server->router);
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

    const char* first_param = argv[1];
    int port = atoi(first_param);
    if (port == 0) {
        printf("Failed to read port number");
        return 1;
    }

    global_server = create_server(port);
    if (!global_server)
        return 1;

    Router* router_ptr = &global_server->router;

    HttpRoute index_route = {
        .method = GET,
        .path = "/",
        .path_length = strlen("/")};
    router_attach_function(router_ptr, &index_route, index_func);

    HttpRoute cat_route = {
        .method = GET,
        .path = "/cat",
        .path_length = strlen("/cat")};
    router_attach_function(router_ptr, &cat_route, cat_func);

    HttpRoute dog_route = {
        .method = GET,
        .path = "/dog",
        .path_length = strlen("/dog")};
    router_attach_function(router_ptr, &dog_route, dog_func);

    signal(SIGINT, handle_sigint);
    run_server(global_server);
    cleanup_server();

    return 0;
}
