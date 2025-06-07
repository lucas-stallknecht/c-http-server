#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

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
    PARSE_FAILED_PARAMS = 4,
    PARSE_FAILED_SERVER = 5,
};

enum route_match_status {
    MATCH_OK = 0,
    MATCH_FAILED = 1,
};

typedef enum http_method HttpMethod;
typedef enum parse_status ParseStatus;
typedef enum route_match_status RouteMatchStatus;

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

typedef void (*ControllerFunc)(char** buffer, size_t* size);

#endif // TYPES_H
