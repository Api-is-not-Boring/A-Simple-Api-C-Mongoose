#include "router.h"

static void print_datetime(mg_pfn_t out, void *ptr) {
    char buff[30];
    time_t t = time(NULL);
    struct tm now = *gmtime(&t);
    strftime(buff, sizeof(buff), "%a, %d %b %Y %T GMT", &now);
    mg_xprintf(out, ptr, "%Q", buff);
}

static void print_stats(mg_pfn_t out, void *ptr, va_list *ap) {
    struct mg_connection *c = va_arg(*ap, struct mg_connection *);
    const char *comma = "";
    for (struct mg_connection *t = c->mgr->conns; t != NULL; t = t->next) {
        mg_xprintf(out, ptr, "%s{%Q:\"%lu\", %Q:\"%s\", %Q:\"%s\", %Q:\"%I:%u\", %Q:\"%I:%u\"}",
                        comma,
                        "id", t->id,
                        "protocol", t->is_udp ? "UDP" : "TCP",
                        "type", t->is_listening  ? "LISTENING": t->is_accepted ? "ACCEPTED ": "CONNECTED",
                        "local", 4, &t->loc.ip, mg_ntohs(t->loc.port),
                        "remote", 4, &t->rem.ip, mg_ntohs(t->rem.port)
        );
        comma = ",";
    }
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
    struct mg_str *agent= mg_http_get_header(hm, "User-Agent");
    struct mg_str endpoint[1];
    mg_match(hm->uri, API_V1, endpoint);
    switch (find_endpoint_index(&endpoint[0], v1_endpoint)) {
        case ping:
            mg_http_reply(c, 200, JSON_TYPE, "{%Q:\"%.*s\",%Q:%Q}",
                          "agent", agent->len, agent->ptr,
                          "message", "pong");
            break;
        case stats:
            mg_http_reply(c, 200, JSON_TYPE, "{%Q:%M,%Q:[%M]}",
                            "date", print_datetime,
                            "connections", print_stats, c);
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
            if (mg_vcasecmp(&hm->method, "GET") == 0) {
                api_v1(c, hm);
            } else {
                mg_http_reply(c, 405, JSON_TYPE, "{%Q:\"%s\"}", "status", "405 Method not allowed");
            }
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
