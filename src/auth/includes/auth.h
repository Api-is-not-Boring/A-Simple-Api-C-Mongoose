#ifndef A_SIMPLE_API_C_MONGOOSE_AUTH_H
#define A_SIMPLE_API_C_MONGOOSE_AUTH_H

#include "mongoose.h"

bool api_auth(mg_pfn_t out, void *ptr, va_list *ap);

char *generate_token(void);

void verify_token(char *token);

#endif //A_SIMPLE_API_C_MONGOOSE_AUTH_H
