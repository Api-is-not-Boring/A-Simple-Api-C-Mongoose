#include <signal.h>
#include "mongoose.h"
#include "model.h"
#include "router.h"
#include "auth.h"


// Handle interrupts, like Ctrl-C
volatile sig_atomic_t s_signum;
static void signal_handler(int signum) {
    s_signum = signum;
}

int main(void) {
    struct mg_mgr mgr;                            // Event manager
    sqlite3 *db;                                     // Database
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    mg_log_set(MG_LL_INFO);                      // Set log level
    db_init(&db);                                  // Initialize database
    mg_mgr_init(&mgr);                            // Initialise event manager
    char *token = generate_token();                              // Generate JWT
    verify_token(token);                          // Verify JWT
    if (mg_http_listen(&mgr, s_http_addr, router, db) == NULL) {
        MG_ERROR(("Cannot listen on %s.", s_http_addr));
        db_close(db);
        exit(EXIT_FAILURE);
    }
    MG_INFO(("Mongoose version : %s", MG_VERSION));
    MG_INFO(("Listening on     : %s", s_http_addr));
    while (!s_signum) mg_mgr_poll(&mgr, 1000);                   // Infinite event loop
    db_close(db);
    mg_mgr_free(&mgr);
    MG_INFO(("Exiting on signal %d", s_signum));
    return 0;
}