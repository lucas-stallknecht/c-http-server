#ifndef ROUTER_H
#define ROUTER_H

#include "types.h"

#define GET_KEY_FROM_ROUTE(route, buffer, buffer_size) \
    snprintf((buffer), (buffer_size), "%d-%s", (route)->method, (route)->path)

// Props to https://xnacly.me/posts/2024/c-hash-map/ for the hashmap implementation.
// The router determines which controller function should handle the request.
// We won't be able to detach functions from the Map though (not a limiting factor).
typedef struct router {
    size_t size;
    size_t capacity;
    void** functions;
} Router;

Router create_router(size_t max_routes);
void destroy_router(Router* router);

size_t hash(const Router* router, char* str);

void attach_function(Router* router, HttpRoute* route, void* function);
RouteMatchStatus get_function(const Router* router, HttpRoute* route, void** function);

#endif // ROUTER_H
