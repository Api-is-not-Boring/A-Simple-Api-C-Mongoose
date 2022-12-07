#ifndef A_SIMPLE_API_C_MONGOOSE_ROUTER_H
#define A_SIMPLE_API_C_MONGOOSE_ROUTER_H

#include "mongoose.h"

#define JSON_TYPE "Content-Type: application/json; charset=utf-8\r\n"
#define API_V1 mg_str("/api/v1/*")
#define FOREACH_V1_ENDPOINT(ENDPOINT) \
    ENDPOINT(ping)               \
    ENDPOINT(stats)                   \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

static const char *s_http_addr = "http://0.0.0.0:8000";

typedef enum v1_enum {
    FOREACH_V1_ENDPOINT(GENERATE_ENUM)
} v1_enum_t;

static const char *v1_endpoint[] = {
        FOREACH_V1_ENDPOINT(GENERATE_STRING)
};

void router(struct mg_connection *c, int event, void *event_data, void *router_data);

#endif //A_SIMPLE_API_C_MONGOOSE_ROUTER_H
