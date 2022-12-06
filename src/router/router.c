#include "router.h"

static size_t print_stat(mg_pfn_t out, void *ptr, va_list *ap) {
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

void router(struct mg_connection *c, int event, void *event_data, void *router_data) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) event_data;
        MG_INFO(("%.*s %.*s %.*s", (int) hm->method.len, hm->method.ptr,
                (int) hm->uri.len, hm->uri.ptr,
                (int) hm->body.len, hm->body.ptr));

        if (mg_http_match_uri(hm, "/api/v1/stats")) {
            if (mg_strstr(hm->uri, mg_str("stat"))) {
                MG_DEBUG(("URI : %s", mg_url_uri(hm->uri.ptr)));
            }
            mg_http_reply(c, 200, JSON_TYPE, "[%M]", print_stat, c);
        } else if (mg_http_match_uri(hm, "/api/v2/*")) {
            if (hm->query.len > 0) {
                MG_DEBUG(
                        ("Request with query: %.*s", (int) hm->query.len, hm->query.ptr));
            }
            char *json = mg_mprintf("{%Q:\"%.*s\"}", "result", (int) hm->uri.len, hm->uri.ptr);
            mg_http_reply(c, 200, JSON_TYPE, "%s\n", json);
            free(json);
        } else {
            mg_http_reply(c, 404, JSON_TYPE, "{%Q:\"%s\"}", "status", "Not found");
        }
    }
    (void) router_data;
}
