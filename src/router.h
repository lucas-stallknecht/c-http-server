#ifndef ROUTER_H
#define ROUTER_H

#include "types.h"

// The router determines which controller function should handle the request

typedef struct controller_entry {
    ControllerFunc function;
    char* key_value;
} ControllerEntry;

typedef struct router {
    size_t size;
    size_t capacity;
    ControllerEntry** entries;
} Router;

size_t _hash(const Router* router, const char* str);

Router create_router(size_t max_routes);
void destroy_router(Router* router);
void router_attach_function(Router* router, const HttpRoute* route, const ControllerFunc function);
RouteMatchStatus router_get_function(const Router* router, const HttpRoute* route, ControllerFunc* function);

#endif // ROUTER_H
