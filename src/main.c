#include <signal.h>
#include "router.h"

// Handle interrupts, like Ctrl-C
static int s_signo;
static void signal_handler(int signo) {
    s_signo = signo;
}

int main(void) {
    struct mg_mgr mgr;                            // Event manager
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    mg_log_set(MG_LL_DEBUG);                      // Set log level
    mg_mgr_init(&mgr);                            // Initialise event manager
    if (mg_http_listen(&mgr, s_http_addr, router, &mgr) == NULL) {
        MG_ERROR(("Cannot listen on %s. Use http://ADDR:PORT or :PORT",
                s_http_addr));
        exit(EXIT_FAILURE);
    }
    MG_INFO(("Mongoose version : %s", MG_VERSION));
    MG_INFO(("Listening on     : %s", s_http_addr));
    while (s_signo == 0) mg_mgr_poll(&mgr, 1000);                   // Infinite event loop
    mg_mgr_free(&mgr);
    MG_INFO(("Exiting on signal %d", s_signo));
    return 0;
}