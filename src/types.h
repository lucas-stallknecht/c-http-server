#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#define NUM_HANDLED_METHODS 2

enum http_method {
    UNKNOWN_METHOD = -1,
    GET = 0,
    POST = 1
};

enum parse_status {
    PARSE_OK = 0,
    PARSE_FAILED_FORMAT = 1,
    PARSE_FAILED_METHOD = 2,
    PARSE_FAILED_PATH = 3,
    PARSE_FAILED_PARAMS = 4
};

typedef enum http_method HttpMethod;
typedef enum parse_status ParseStatus;

struct http_route {
    enum http_method method;
    char* path;
    size_t path_length;
};

struct parse_result {
    enum parse_status status;
    struct http_route requested_route;
    void* params; // Will be defined later
};

typedef struct http_route HttpRoute;
typedef struct parse_result ParseResult;

#endif // TYPES_H
