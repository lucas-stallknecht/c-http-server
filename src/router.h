#ifndef ROUTER_H
#define ROUTER_H

#include "types.h"

#define GET_KEY_FROM_ROUTE(route, buffer, buffer_size) \
    snprintf((buffer), (buffer_size), "%d-%s", (route)->method, (route)->path)

// Props to https://xnacly.me/posts/2024/c-hash-map/ for the hashmap implementation.
// The router determines which controller function should handle the request.
typedef struct router {
    size_t size;
    size_t capacity;
    ControllerFunc* functions;
} Router;
// TODO improve this data structure

size_t _hash(const Router* router, char* str);

Router create_router(size_t max_routes);
void destroy_router(Router* router);
void router_attach_function(Router* router, HttpRoute* route, ControllerFunc function);
RouteMatchStatus router_get_function(const Router* router, HttpRoute* route, ControllerFunc* function);

#endif // ROUTER_H
