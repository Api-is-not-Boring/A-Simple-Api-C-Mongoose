#include <sqlite3.h>
#include "auth.h"
#include "l8w8jwt/encode.h"
#include "l8w8jwt/decode.h"
#include "project.h"

bool api_auth(mg_pfn_t out, void *ptr, va_list *ap) {
    sqlite3 *db = va_arg(*ap, sqlite3 *);
    char *username = va_arg(*ap, char *);
    char *password = va_arg(*ap, char *);
    mg_bcrypt_salt salt;
    mg_bcrypt_gen_salt(salt);
    sqlite3_stmt *stmt;
    char *sql_select = "SELECT * FROM Users WHERE Username = ?";
    sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW && mg_bcrypt_verify_pw(password, (char *) sqlite3_column_text(stmt, 2)) == 0) {
        mg_xprintf(out, ptr, "%Q:%Q,%Q:%Q",
                   "message", "Login Successful !!!",
                   "token", generate_token());
    } else {
        mg_xprintf(out, ptr, "%Q:%Q", "error", "Invalid username or password");
    }
    sqlite3_finalize(stmt);
    return true;
}

char *generate_token(void) {
    char* jwt;
    size_t jwt_length;
    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.iat = time(NULL);
    params.exp = time(NULL) + 60; /* Set to expire after 1 minutes (60 seconds). */

    params.secret_key = (unsigned char*) SECRET;
    params.secret_key_length = strlen((const char* ) params.secret_key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);
    if (r != L8W8JWT_SUCCESS) {
        MG_ERROR(("Error while encoding JWT: %d", r));
    } else {
        MG_INFO(("JWT: %s", jwt));
    }
    return jwt;
}

bool verify_token(char *token) {
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.jwt = token;
    params.jwt_length = strlen(token);

    params.verification_key = (unsigned char*) SECRET;
    params.verification_key_length = strlen(SECRET);

    params.validate_exp = 1;
    params.exp_tolerance_seconds = 60;

    enum l8w8jwt_validation_result validation_result;

    int decode_result = l8w8jwt_decode(&params,
                                       &validation_result,
                                       NULL,
                                       NULL
                                       );

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) {
        MG_INFO(("JWT HS512 token validation successful!"));
        return true;
    } else {
        MG_ERROR(("Error while decoding JWT"));
        return false;
    }
}
