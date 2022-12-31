#include "auth.h"
#include "ow-crypt.h"

void mg_bcrypt_gen_salt(mg_bcrypt_salt salt) {
    char random_buf[16];
    mg_random(random_buf, sizeof(random_buf));
    crypt_gensalt_rn("$2b$", 12, random_buf, 16,
                     salt, BCRYPT_HASH_SIZE);
};

void mg_bcrypt_hash_pw(const char *password, mg_bcrypt_salt salt, mg_bcrypt_hash hash) {
    crypt_rn(password, salt, hash, BCRYPT_HASH_SIZE);
};

int mg_bcrypt_verify_pw(const char *password, mg_bcrypt_hash hash) {
    mg_bcrypt_hash out_hash;
    mg_bcrypt_hash_pw(password, hash, out_hash);
    return mg_casecmp(out_hash, hash);
};
