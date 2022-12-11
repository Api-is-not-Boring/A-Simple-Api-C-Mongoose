#include "router.h"
#include "project.h"
#include "model.h"


static void print_datetime(mg_pfn_t out, void *ptr) {
    char *datetime = malloc(sizeof(char) * 35);
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    strftime(datetime, 35, "%a %d %b %Y %T GMT%z", &now);
    mg_xprintf(out, ptr, "%Q", datetime);
    free(datetime);
}

static char *print_header(void) {
    char *datetime = malloc(sizeof(char) * 30);
    time_t t = time(NULL);
    struct tm now = *gmtime(&t);
    strftime(datetime, 30, "%a, %d %b %Y %T GMT", &now);
    return mg_mprintf(SERVER "Date: %s\r\n" JSON_TYPE, datetime);
}

static inline void print_project_info(mg_pfn_t out, void *ptr) {
    mg_xprintf(out, ptr, "{%Q:%Q,%Q:%Q,%Q:%Q,%Q:%Q,%Q:%Q,%Q:%Q}",
               "name", PROJECT_NAME,
               "description", PROJECT_DESCRIPTION,
               "language", PROJECT_LANGUAGE,
               "url", PROJECT_URL,
               "git hash", GIT_HASH,
               "version", API_VERSION
    );
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
        if (v[i] != NULL && !mg_vcmp(endpoint, v[i])) {
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
            mg_http_reply(c, 200, print_header(), "{%Q:\"%.*s\",%Q:%M,%Q:%Q}",
                          "agent", agent->len, agent->ptr,
                          "datetime", print_datetime,
                          "message", "pong");
            break;
        case info:
            mg_http_reply(c, 200, print_header(), "{%Q:%M}",
                          "project", print_project_info);
            break;
        case connections:
            mg_http_reply(c, 200, print_header(), "{%Q:[%M]}",
                          "connections", print_stats, c);
            break;
        default:
            mg_http_reply(c, 404, print_header(), "{%Q:\"%s\"}",
                          "status", "404 Not found");
            break;
    }
}

static void api_v2(struct mg_connection *c, struct mg_http_message *hm, sqlite3 *db){
    char *end;
    struct mg_str endpoint[1];
    struct mg_str *content_type = mg_http_get_header(hm, "Content-Type");
    if (mg_http_match_uri(hm, "/api/v2/cars")) {
        switch (find_endpoint_index(&hm->method, http_method)) {
            case GET:
                if (hm->query.len) {
                    struct mg_str v = mg_http_var(hm->query, mg_str("id"));
                    int id = v.ptr == NULL ? 0 : (int) strtol(v.ptr, &end, 10);
                    if (v.len && id) {
                        mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                                      "method", "GET with Query Parameter",
                                      db_get_car_by_id, db, id);
                    } else goto bad_request;
                } else goto all_cars;
                break;
            case POST:
                if (!mg_vcmp(content_type, "application/json")) {
                    char *name = mg_json_get_str(hm->body, "$.name");
                    long price = mg_json_get_long(hm->body, "$.price", 0);
                    if (db_create_car(db, name,(int) price)) {
                        int last_id = (int) sqlite3_last_insert_rowid(db);
                        mg_http_reply(c, 201, print_header(), "{%Q:%Q,%M}",
                                      "method", "POST with JSON",
                                      db_get_car_by_id, db, last_id);
                    } else goto bad_request;
                } else goto bad_request;
                break;
            case PUT:
                if (!mg_vcmp(content_type, "application/json")) {
                    int id = (int) mg_json_get_long(hm->body, "$.id", 0);
                    char *name = mg_json_get_str(hm->body, "$.name");
                    long price = mg_json_get_long(hm->body, "$.price", 0);
                    if (db_update_car(db, id, name, (int) price)) {
                        mg_http_reply(c, 200, print_header(), "{%Q:%Q,%Q:%M}",
                                      "method", "PUT with JSON",
                                      "car", db_get_car_by_id, db, id);
                    } else goto bad_request;
                } else goto bad_request;
                break;
            default:
                goto not_allowed;
                break;
        }

    }
    if (mg_match(hm->uri, mg_str("/api/v2/cars/*"), endpoint)) {
        int id = (int) strtol(endpoint[0].ptr, &end, 10);
        if (db_car_is_existed(db, id)) {
            switch (find_endpoint_index(&hm->method, http_method)) {
                case GET:
                    goto get_car;
                case PUT:
                    if (!mg_vcmp(content_type, "application/json")) {
                        char *name = mg_json_get_str(hm->body, "$.name");
                        long price = mg_json_get_long(hm->body, "$.price", 0);
                        if (db_update_car(db, id, name, (int) price)) {
                            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                                          "method", "PUT with JSON",
                                          db_get_car_by_id, db, id);
                        } else goto bad_request;
                    } else goto bad_request;
                    break;
                case DELETE:
                    if (db_delete_car(db, id)) {
                        mg_http_reply(c, 200, print_header(), "{%Q:%Q,%Q:%M}",
                                      "method", "DELETE with Path Parameter",
                                      "total", db_count_all_cars, db);
                    } else goto bad_request;
                    break;
                default:
                    goto not_allowed;
                    break;
            }
        } else goto get_car;
        get_car:
        mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                      "method", "GET with Path Parameter",
                      db_get_car_by_id, db, id);
    } else goto not_found;

    all_cars:
    mg_http_reply(c, 200, print_header(), "{%Q:%M,%Q:[%M]}",
                  "total", db_count_all_cars, db,
                  "cars", db_get_all_cars, db);
    bad_request:
    mg_http_reply(c, 400, print_header(),
                  "{%Q:%Q}", "status", "400 Bad request");
    not_allowed:
    mg_http_reply(c, 405, print_header(),
                  "{%Q:%Q}", "status", "405 Method not allowed");
    not_found:
    mg_http_reply(c, 404, print_header(),
                  "{%Q:%Q}", "status", "404 Not found");
}

void router(struct mg_connection *c, int event, void *event_data, void *db) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) event_data;
        MG_INFO(("%.*s %.*s %.*s",
                (int) hm->method.len, hm->method.ptr,
                (int) hm->uri.len, hm->uri.ptr,
                (int) hm->body.len, hm->body.ptr));

        if (mg_http_match_uri(hm, "/api/v1/*")) {
            if (!mg_vcasecmp(&hm->method, "GET")) {
                api_v1(c, hm);
            } else goto not_allowed;
        } else if (mg_http_match_uri(hm, "/api/v2/*") || mg_http_match_uri(hm, "/api/v2/*/*")) {
            api_v2(c, hm, db);
        } else goto not_found;

        not_allowed:
        mg_http_reply(c, 405, print_header(),
                      "{%Q:%Q}", "status", "405 Method not allowed");
        not_found:
        mg_http_reply(c, 404, print_header(),
                      "{%Q:%Q}", "status", "404 Not found");
    }
}
