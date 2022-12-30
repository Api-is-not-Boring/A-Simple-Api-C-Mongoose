#include "model.h"
#include "auth.h"

char *sql = "DROP TABLE IF EXISTS Cars;"
            "CREATE TABLE Cars("
            "Id    integer constraint Car_pk primary key autoincrement,"
            "Name  TEXT not null on conflict abort,"
            "Price INT  not null on conflict abort);"
            "INSERT INTO Cars (Name, Price) VALUES('Audi', 52642);"
            "INSERT INTO Cars (Name, Price) VALUES('Mercedes', 57127);"
            "INSERT INTO Cars (Name, Price) VALUES('Skoda', 9000);"
            "INSERT INTO Cars (Name, Price) VALUES('Volvo', 29000);"
            "INSERT INTO Cars (Name, Price) VALUES('Bentley', 350000);"
            "INSERT INTO Cars (Name, Price) VALUES('Citroen', 21000);"
            "INSERT INTO Cars (Name, Price) VALUES('Hummer', 41400);"
            "INSERT INTO Cars (Name, Price) VALUES('Volkswagen', 21600);";

char *sql_auth = "DROP TABLE IF EXISTS Users;"
                 "CREATE TABLE Users("
                 "Id    integer constraint User_pk primary key autoincrement,"
                 "Username  TEXT not null on conflict abort,"
                 "Password  TEXT not null on conflict abort);";

void db_init(sqlite3 **db) {
    char *err_msg = 0;
    int rc = sqlite3_open(":memory:", db);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot open database: %s\n", sqlite3_errmsg(*db)));
        sqlite3_close(*db);
        exit(EXIT_FAILURE);
    }
    MG_INFO(("Sqlite version : %s", sqlite3_libversion()));

    rc = sqlite3_exec(*db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        MG_ERROR(("SQL error: %s\n", err_msg));
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_exec(*db, sql_auth, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        MG_ERROR(("SQL error: %s\n", err_msg));
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        exit(EXIT_FAILURE);
    }

    mg_bcrypt_salt salt;
    mg_bcrypt_hash hash;
    mg_bcrypt_gen_salt(salt);
    mg_bcrypt_hash_pw("password", salt, hash);
    char *sql_create_user = mg_mprintf("INSERT INTO Users (Username, Password) VALUES('admin', '%s');", hash);
    MG_INFO(("%s", sql_create_user));
    rc = sqlite3_exec(*db, sql_create_user, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        MG_ERROR(("SQL error: %s\n", err_msg));
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        exit(EXIT_FAILURE);
    }
}

inline void db_close(sqlite3 *db) {
    sqlite3_close(db);
    MG_INFO(("Database connection terminated !!!"));
}

static void db_reset(sqlite3 *db) {
    char *err_msg = 0;
    MG_INFO(("Database reset !!!"));
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        MG_ERROR(("SQL error: %s\n", err_msg));
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
}

void db_count_all_cars(mg_pfn_t out, void *ptr, va_list *ap) {
    sqlite3 *db = va_arg(*ap, sqlite3 *);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Cars;", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot prepare statement: %s\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        MG_ERROR(("Cannot step: %s\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    if (sqlite3_column_int(stmt, 0) <= 0) {
        sqlite3_finalize(stmt);
        db_reset(db);
        rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Cars;", -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            MG_ERROR(("Cannot prepare statement: %s\n", sqlite3_errmsg(db)));
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            MG_ERROR(("Cannot step: %s\n", sqlite3_errmsg(db)));
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }
    }
    mg_xprintf(out, ptr, "%d", sqlite3_column_int(stmt, 0));
    sqlite3_finalize(stmt);
}

void db_get_all_cars(mg_pfn_t out, void *ptr, va_list *ap) {
    sqlite3 *db = va_arg(*ap, sqlite3 *);
    sqlite3_stmt *res;
    const char *comma = "";
    char *sql_select_all = "SELECT * FROM Cars";
    sqlite3_prepare_v2(db, sql_select_all, -1, &res, 0);
    int step = sqlite3_step(res);
    do {
        if (step == SQLITE_ROW) {
            mg_xprintf(out, ptr, "%s{%Q:%d,%Q:%Q,%Q:%d}",
                       comma,
                       "id", sqlite3_column_int(res, 0),
                       "name", sqlite3_column_text(res, 1),
                       "price", sqlite3_column_int(res, 2));
        }
        comma = ",";
        step = sqlite3_step(res);
    } while (step == SQLITE_ROW);
    sqlite3_finalize(res);
}

void db_get_car_by_id(mg_pfn_t out, void *ptr, va_list *ap) {
    sqlite3 *db = va_arg(*ap, sqlite3 *);
    int id = va_arg(*ap, int);
    sqlite3_stmt *res;
    char *sql_select = "SELECT * FROM Cars WHERE Id = ?";
    sqlite3_prepare_v2(db, sql_select, -1, &res, 0);
    sqlite3_bind_int(res, 1, id);
    int step = sqlite3_step(res);
    if (step == SQLITE_ROW) {
        mg_xprintf(out, ptr, "%Q:{%Q:%d,%Q:%Q,%Q:%d}",
                   "car",
                   "id", sqlite3_column_int(res, 0),
                   "name", sqlite3_column_text(res, 1),
                   "price", sqlite3_column_int(res, 2));
    } else {
        mg_xprintf(out, ptr, "%Q:%Q", "error", "car not found");
    }
    sqlite3_finalize(res);
}

bool db_car_is_existed(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Cars WHERE Id = ?;", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot prepare statement: %s\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot bind id: %s\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        MG_ERROR(("Cannot step: %s\n", sqlite3_errmsg(db)));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    bool exist = sqlite3_column_int(stmt, 0) > 0;
    sqlite3_finalize(stmt);
    return exist;
}

bool db_create_car(sqlite3 *db, const char *name, int price) {
    int last_id = (int) sqlite3_last_insert_rowid(db);
    if (last_id >= 20) db_reset(db);
    sqlite3_stmt *res;
    char *sql_create = "INSERT INTO Cars (Name, Price) VALUES(?, ?);";
    int rc = sqlite3_prepare_v2(db, sql_create, -1, &res, 0);
    sqlite3_bind_text(res, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(res, 2, price);
    sqlite3_step(res);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot step: %s | Code: %d", sqlite3_errmsg(db), rc));
        return false;
    }
    sqlite3_finalize(res);
    return true;
}

bool db_update_car(sqlite3 *db, int id, const char *name, int price) {
    sqlite3_stmt *res;
    char *sql_update = "UPDATE Cars SET Name = ?, Price = ? WHERE Id = ?;";
    int rc = sqlite3_prepare_v2(db, sql_update, -1, &res, 0);
    sqlite3_bind_text(res, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(res, 2, price);
    sqlite3_bind_int(res, 3, id);
    sqlite3_step(res);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot step: %s | Code: %d", sqlite3_errmsg(db), rc));
        return false;
    }
    sqlite3_finalize(res);
    return true;
}

bool db_delete_car(sqlite3 *db, int id) {
    sqlite3_stmt *res;
    char *sql_delete = "DELETE FROM Cars WHERE Id = ?;";
    int rc = sqlite3_prepare_v2(db, sql_delete, -1, &res, 0);
    sqlite3_bind_int(res, 1, id);
    sqlite3_step(res);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot step: %s | Code: %d", sqlite3_errmsg(db), rc));
        return false;
    }
    sqlite3_finalize(res);
    return true;
}
