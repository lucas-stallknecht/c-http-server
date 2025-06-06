#include "router.h"
#include "types.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_KEY_FROM_ROUTE(route, buffer, buffer_size) \
    snprintf((buffer), (buffer_size), "%d-%s", (route)->method, (route)->path)

static const size_t HASH_BASE = 0x811c9dc5;
static const size_t HASH_PRIME = 0x01000193;

size_t _hash(const Router* router, const char* str) {
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
    router.entries = malloc(sizeof(ControllerEntry*) * router.capacity);
    if (router.entries) {
        memset(router.entries, 0, sizeof(ControllerEntry*) * router.capacity);
        for (int i = 0; i < router.capacity; i++) {
            router.entries[i] = malloc(sizeof(ControllerEntry));
            if (router.entries[i]) {
                router.entries[i]->function = NULL;
                router.entries[i]->key_value = NULL;
            };
        }
    }

    return router;
}

void destroy_router(Router* router) {
    if (!router || !router->entries)
        return;

    for (size_t i = 0; i < router->capacity; ++i) {
        if (router->entries[i]) {
            free(router->entries[i]->key_value);
            free(router->entries[i]);
        }
    }

    free(router->entries);
    router->entries = NULL;
    router->size = 0;
    router->capacity = 0;
}

void router_attach_function(Router* router, const HttpRoute* route, const ControllerFunc function) {
    assert(route->path_length > 0);
    assert(route->method != -1);
    assert(route->path != NULL);

    if (router->size == router->capacity)
        return; // TODO? resize hash table

    char key[route->path_length + 3];
    GET_KEY_FROM_ROUTE(route, key, sizeof(key));

    const size_t hash_code = _hash(router, key);

    // Collision handling with linear probing
    int i = hash_code;
    while (router->entries[i]->function != NULL) {
        i = (i + 1) % router->capacity;
        if (i == hash_code) {
            // Table is full
            return;
        }
    }
    router->entries[i]->function = function;
    // Will actually create memory on the Heap and copy the key
    router->entries[i]->key_value = strdup(key);
    router->size++;
}

RouteMatchStatus router_get_function(const Router* router, const HttpRoute* route, ControllerFunc* function) {
    char key[route->path_length + 3];
    GET_KEY_FROM_ROUTE(route, key, sizeof(key));

    const size_t hash_code = _hash(router, key);
    int i = hash_code;

    while (router->entries[i]->function != NULL) {
        if (strcmp(key, router->entries[i]->key_value) == 0) {
            *function = router->entries[i]->function;
            return MATCH_OK;
        }

        i = (i + 1) % router->capacity;
        if (i == hash_code)
            break; // Full cycle, no match
    }

    *function = NULL;
    return MATCH_FAILED;
}
