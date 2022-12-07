#include "router.h"

static size_t print_stats(mg_pfn_t out, void *ptr, va_list *ap) {
    struct mg_connection *c = va_arg(*ap, struct mg_connection*);
    const char *comma = "";
    size_t n = 0;
    for (struct mg_connection *t = c->mgr->conns; t != NULL; t = t->next) {
        n += mg_xprintf(out, ptr, "%s{%Q:\"%lu\", %Q:\"%s\", %Q:\"%s\", %Q:\"%I:%u\", %Q:\"%I:%u\"}",
                        comma,
                        "id", t->id,
                        "protocol", t->is_udp ? "UDP" : "TCP",
                        "type", t->is_listening  ? "LISTENING": t->is_accepted ? "ACCEPTED ": "CONNECTED",
                        "local", 4, &t->loc.ip, mg_ntohs(t->loc.port),
                        "remote", 4, &t->rem.ip, mg_ntohs(t->rem.port)
        );
        comma = ",";
    }
    return n;
}

static int find_endpoint_index(struct mg_str *endpoint,const char *v[]) {
    for (int i = 0; i < sizeof(&v) / sizeof(*v[0]); i++) {
        if (mg_vcmp(endpoint, v[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static void api_v1(struct mg_connection *c, struct mg_http_message *hm){
    struct mg_str endpoint[1];
    mg_match(hm->uri, API_V1, endpoint);
    switch (find_endpoint_index(&endpoint[0], v1_endpoint)) {
        case ping:
            mg_http_reply(c, 200, JSON_TYPE, "{%Q:%Q}", "message", "pong");
            break;
        case stats:
            mg_http_reply(c, 200, JSON_TYPE, "{%Q:[%M]}", "connections", print_stats, c);
            break;
        default:
            mg_http_reply(c, 404, JSON_TYPE, "{%Q:\"%s\"}", "status", "404 Not found");
            break;
    }
}

void router(struct mg_connection *c, int event, void *event_data, void *router_data) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) event_data;
        MG_INFO(("%.*s %.*s %.*s", (int) hm->method.len, hm->method.ptr,
                (int) hm->uri.len, hm->uri.ptr,
                (int) hm->body.len, hm->body.ptr));

        if (mg_http_match_uri(hm, "/api/v1/*")) {
            api_v1(c, hm);
        } else if (mg_http_match_uri(hm, "/api/v2/*")) {
            if (hm->query.len > 0) {
                MG_DEBUG(
                        ("Request with query: %.*s", (int) hm->query.len, hm->query.ptr));
            }
            char *json = mg_mprintf("{%Q:\"%.*s\"}", "result", (int) hm->uri.len, hm->uri.ptr);
            mg_http_reply(c, 200, JSON_TYPE, "%s\n", json);
            free(json);
        } else {
            mg_http_reply(c, 404, JSON_TYPE, "{%Q:\"%s\"}", "status", "404 Not found");
        }
    }
    (void) router_data;
}
