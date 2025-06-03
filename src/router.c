#include "router.h"
#include "types.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t HASH_BASE = 0x811c9dc5;
const size_t HASH_PRIME = 0x01000193;

size_t hash(const Router* router, char* str) {
    size_t initial = HASH_BASE;

    while (*str) {
        initial ^= *str++;
        initial *= HASH_PRIME;
    }
    // in [0, capacity - 1]
    return initial & (router->capacity - 1);
}

Router create_router(size_t max_routes) {
    Router router = {
        .size = 0,
        .capacity = max_routes,
    };
    router.functions = malloc(sizeof(void*) * router.capacity);
    if (router.functions)
        memset(router.functions, 0, sizeof(void*) * router.capacity);

    return router;
}

void destroy_router(Router* router) {
    if (!router)
        return;

    free(router->functions);
    router->functions = NULL;
    router->size = 0;
    router->capacity = 0;
}

void attach_function(Router* router, HttpRoute* route, void* function) {
    assert(route->path_length > 0);
    assert(route->method != -1);
    assert(route->path != NULL);

    if (router->size == router->capacity)
        return; // TODO: proper warning

    char key[route->path_length];
    GET_KEY_FROM_ROUTE(route, key, route->path_length);

    router->functions[hash(router, key)] = function;
    router->size++;
}

RouteMatchStatus get_function(const Router* router, HttpRoute* route, void** function) {
    char key[route->path_length];
    GET_KEY_FROM_ROUTE(route, key, route->path_length);

    *function = router->functions[hash(router, key)];

    if (function == NULL)
        return MATCH_FAILED;

    return MATCH_OK;
}
