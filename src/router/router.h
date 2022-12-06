#ifndef A_SIMPLE_API_C_MONGOOSE_ROUTER_H
#define A_SIMPLE_API_C_MONGOOSE_ROUTER_H

#include "mongoose.h"

#define JSON_TYPE "Content-Type: application/json\r\n"

static const char *s_http_addr = "http://0.0.0.0:8000";

void router(struct mg_connection *c, int event, void *event_data, void *router_data);

#endif //A_SIMPLE_API_C_MONGOOSE_ROUTER_H
