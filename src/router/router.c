#include "router.h"
#include "project.h"
#include "model.h"
#include "auth.h"


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
    return mg_mprintf(SERVER "Date: %s\r\n" JSON_TYPE_HEADER, datetime);
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

static int find_enum_index(struct mg_str *endpoint,const char *v[]) {
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
    switch (find_enum_index(&endpoint[0], v1_endpoint)) {
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
        // Request Methods for API v2 with Query Parameter
        if (hm->query.len) {
            struct mg_str v = mg_http_var(hm->query, mg_str("id"));
            int id = v.ptr == NULL ? 0 : (int) strtol(v.ptr, &end, 10);
            if (!db_car_is_existed(db, id)) goto v2_not_found;
            switch (find_enum_index(&hm->method, http_method)) {
                case GET:
                    if (v.len && id) goto v2_get_car_with_query; else goto v2_bad_request;
                case PUT:
                    if (!mg_vcmp(content_type, "application/json")) {
                        char *name = mg_json_get_str(hm->body, "$.name");
                        long price = mg_json_get_long(hm->body, "$.price", 0);
                        if (db_update_car(db, id, name, (int) price)) goto v2_update_car_with_query;
                        else goto v2_internal_error;
                    } else goto v2_bad_request;
                    case DELETE:
                    if (db_delete_car(db, id)) goto v2_delete_car_with_query; else goto v2_internal_error;
                default:
                    goto v2_method_not_allowed;
            }
            // Response for API v2 from Query Parameter =======================================================
            v2_get_car_with_query:
            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                          "method", "GET with Query Parameter",
                          db_get_car_by_id, db, id);
            v2_update_car_with_query:
            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                          "method", "PUT with JSON in Path Parameter",
                          db_get_car_by_id, db, id);
            v2_delete_car_with_query:
            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%Q:%d,%Q:%M}",
                          "method", "DELETE with Query Parameter",
                          "deleted", id,
                          "total", db_count_all_cars, db);

        } else switch (find_enum_index(&hm->method, http_method)) {
            case GET:
                goto v2_all_cars;
            case POST:
                goto v2_create_car;
            case PUT:
                goto v2_update_car;
            default:
                goto v2_method_not_allowed;
        }
    }
    if (mg_match(hm->uri, mg_str("/api/v2/cars/*"), endpoint)) {
        int id = (int) strtol(endpoint[0].ptr, &end, 10);
        if (db_car_is_existed(db, id)) {
            switch (find_enum_index(&hm->method, http_method)) {
                case GET:
                    goto v2_get_car_with_path;
                case PUT:
                    goto v2_update_car_with_path;
                case DELETE:
                    goto v2_delete_car_with_path;
                default:
                    goto v2_method_not_allowed;
            }
        } else goto v2_not_found;

        // Response for API v2 from Path Parameter ===========================================================
        v2_get_car_with_path:
        mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                      "method", "GET with Path Parameter",
                      db_get_car_by_id, db, id);
        return;
        v2_update_car_with_path:
        if (!mg_vcmp(content_type, "application/json")) {
            char *name = mg_json_get_str(hm->body, "$.name");
            long price = mg_json_get_long(hm->body, "$.price", 0);
            if (db_update_car(db, id, name, (int) price)) {
                mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                              "method", "PUT with JSON in Path Parameter",
                              db_get_car_by_id, db, id);
            } else goto v2_internal_error;
        } else goto v2_bad_request;
        v2_delete_car_with_path:
        if (db_delete_car(db, id)) {
            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%Q:%d,%Q:%M}",
                          "method", "DELETE with Path Parameter",
                          "deleted_id", id,
                          "total", db_count_all_cars, db);
        } else goto v2_internal_error;
    } else goto v2_not_found;

    // Response for API v2 from Endpoint =======================================================================
    v2_all_cars:
    mg_http_reply(c, 200, print_header(), "{%Q:%M,%Q:[%M]}",
                  "total", db_count_all_cars, db,
                  "cars", db_get_all_cars, db);
    return;
    v2_create_car:
    if (!mg_vcmp(content_type, "application/json")) {
        char *name = mg_json_get_str(hm->body, "$.name");
        long price = mg_json_get_long(hm->body, "$.price", 0);
        if (db_create_car(db, name,(int) price)) {
            int last_id = (int) sqlite3_last_insert_rowid(db);
            mg_http_reply(c, 201, print_header(), "{%Q:%Q,%M}",
                          "method", "POST with JSON",
                          db_get_car_by_id, db, last_id);
        } else goto v2_bad_request;
    } else goto v2_bad_request;
    v2_update_car:
    if (!mg_vcmp(content_type, "application/json")) {
        int id = (int) mg_json_get_long(hm->body, "$.id", 0);
        if (!db_car_is_existed(db, id)) goto v2_not_found;
        char *name = mg_json_get_str(hm->body, "$.name");
        long price = mg_json_get_long(hm->body, "$.price", 0);
        if (db_update_car(db, id, name, (int) price)) {
            mg_http_reply(c, 200, print_header(), "{%Q:%Q,%M}",
                          "method", "PUT with JSON",
                          db_get_car_by_id, db, id);
        } else goto v2_internal_error;
    } else goto v2_bad_request;

    // Error Response for API v2 ==============================================================================
    v2_bad_request:
    mg_http_reply(c, 400, print_header(),
                  "{%Q:%Q}", "status", "400 Bad request");
    return;
    v2_method_not_allowed:
    mg_http_reply(c, 405, print_header(),
                  "{%Q:%Q}", "status", "405 Method not allowed");
    return;
    v2_not_found:
    mg_http_reply(c, 404, print_header(),
                  "{%Q:%Q}", "status", "404 Not found");
    return;
    v2_internal_error:
    mg_http_reply(c, 500, print_header(),
                  "{%Q:%Q}", "status", "500 Internal server error");
}

void api_v3(struct mg_connection *c, struct mg_http_message *hm, sqlite3 *db) {
    struct mg_str endpoint[1];
    mg_match(hm->uri, API_V3, endpoint);
    switch (find_enum_index(&endpoint[0], v3_endpoint)) {
        case auth:
            if (!mg_vcasecmp(&hm->method, "GET")) {
                mg_http_reply(c, 200, print_header(), "{%Q:%Q}",
                              "message", "Login with Post Request");
            } else {
                char *username, *password;
                struct mg_http_part part;
                size_t ofs = 0;
                while ((ofs = mg_http_next_multipart(hm->body, ofs, &part)) > 0) {
                    if (!mg_vcmp(&part.name, "username")) {
                        username = mg_mprintf("%.*s", (int) part.body.len, part.body.ptr);
                        MG_INFO(("User: [%.*s]", (int) part.body.len, part.body.ptr));
                    }
                    if (!mg_vcmp(&part.name, "password")) {
                        password = mg_mprintf("%.*s", (int) part.body.len, part.body.ptr);
                        MG_INFO(("Pass: [%.*s]", (int) part.body.len, part.body.ptr));
                    }
                }
                mg_http_reply(c, 200, print_header(), "{%M}",
                              api_auth, db, username, password);
                free(username);
                free(password);
            }
            break;
        case check:
            break;
        case secure:
            break;
        default:
            mg_http_reply(c, 401, print_header(), "{%Q:\"%s\"}",
                          "status", "401 Unauthorized");
            break;
    }
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
            } else goto router_method_not_allowed;
        } else if (mg_http_match_uri(hm, "/api/v2/*") || mg_http_match_uri(hm, "/api/v2/*/*")) {
            api_v2(c, hm, db);
        } else if (mg_http_match_uri(hm, "/api/v3/*")){
            if (!mg_vcasecmp(&hm->method, "GET") || !mg_vcasecmp(&hm->method, "POST")) {
                api_v3(c, hm, db);
            } else goto router_method_not_allowed;
        } else goto router_not_found;

        router_method_not_allowed:
        mg_http_reply(c, 405, print_header(),
                      "{%Q:%Q}", "status", "405 Method not allowed");
        return;
        router_not_found:
        mg_http_reply(c, 404, print_header(),
                      "{%Q:%Q}", "status", "404 Not found");
        return;
    }
}
