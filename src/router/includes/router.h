#ifndef A_SIMPLE_API_C_MONGOOSE_ROUTER_H
#define A_SIMPLE_API_C_MONGOOSE_ROUTER_H

#include "mongoose.h"

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define FOREACH_HTTP_METHOD(METHOD) \
    METHOD(GET) \
    METHOD(POST) \
    METHOD(PUT) \
    METHOD(DELETE) \

typedef enum http_method {
    FOREACH_HTTP_METHOD(GENERATE_ENUM)
} http_method_t;

static const char *http_method[] = {
    FOREACH_HTTP_METHOD(GENERATE_STRING)
};

#define API_V1 mg_str("/api/v1/*")
#define FOREACH_V1_ENDPOINT(ENDPOINT) \
    ENDPOINT(ping)                   \
    ENDPOINT(info)                    \
    ENDPOINT(connections)              \

typedef enum v1_enum {
    FOREACH_V1_ENDPOINT(GENERATE_ENUM)
} v1_enum_t;

static const char *v1_endpoint[] = {
        FOREACH_V1_ENDPOINT(GENERATE_STRING)
};

#define API_V3 mg_str("/api/v3/*")
#define FOREACH_V3_ENDPOINT(ENDPOINT) \
    ENDPOINT(auth)                   \
    ENDPOINT(check)                    \
    ENDPOINT(secure)                  \

typedef enum v3_enum {
    FOREACH_V3_ENDPOINT(GENERATE_ENUM)
} v3_enum_t;

static const char *v3_endpoint[] = {
        FOREACH_V3_ENDPOINT(GENERATE_STRING)
};

#define SERVER "Server: Mongoose\r\n"
#define JSON_TYPE "application/json"
#define JSON_TYPE_HEADER "Content-Type: "JSON_TYPE"\r\n"
#define REDIRECT_HEADER SERVER"Location: /api/v1/info\r\n"JSON_TYPE_HEADER

static const char *s_http_addr = "http://0.0.0.0:8000";

void router(struct mg_connection *c, int event, void *event_data, void *router_data);

#endif //A_SIMPLE_API_C_MONGOOSE_ROUTER_H
