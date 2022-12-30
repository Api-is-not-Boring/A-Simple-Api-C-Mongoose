#ifndef A_SIMPLE_API_C_MONGOOSE_AUTH_H
#define A_SIMPLE_API_C_MONGOOSE_AUTH_H

#include "mongoose.h"

#define BCRYPT_HASH_SIZE 64

typedef char mg_bcrypt_salt[BCRYPT_HASH_SIZE];
typedef char mg_bcrypt_hash[BCRYPT_HASH_SIZE];

void mg_bcrypt_gen_salt(mg_bcrypt_salt salt);

void mg_bcrypt_hash_pw(const char *password, mg_bcrypt_salt salt, mg_bcrypt_hash hash);

bool mg_bcrypt_verify_pw(const char *password, mg_bcrypt_hash hash);

bool api_auth(mg_pfn_t out, void *ptr, va_list *ap);

char *generate_token(void);

bool verify_token(char *token);

#endif //A_SIMPLE_API_C_MONGOOSE_AUTH_H
