#include "model.h"

void db_init(sqlite3 **db) {
    char *err_msg = 0;
    int rc = sqlite3_open(":memory:", db);
    if (rc != SQLITE_OK) {
        MG_ERROR(("Cannot open database: %s\n", sqlite3_errmsg(*db)));
        sqlite3_close(*db);
        exit(EXIT_FAILURE);
    }
    MG_INFO(("Sqlite version : %s", sqlite3_libversion()));
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

    rc = sqlite3_exec(*db, sql, 0, 0, &err_msg);

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

void db_get_all_cars(mg_pfn_t out, void *ptr, va_list *ap) {
    sqlite3 *db = va_arg(*ap, sqlite3 *);
    sqlite3_stmt *res;
    const char *comma = "";
    char *sql = "SELECT * FROM Cars";
    sqlite3_prepare_v2(db, sql, -1, &res, 0);
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
    char *sql = "SELECT * FROM Cars WHERE Id = ?";
    sqlite3_prepare_v2(db, sql, -1, &res, 0);
    sqlite3_bind_int(res, 1, id);
    int step = sqlite3_step(res);
    if (step == SQLITE_ROW) {
        mg_xprintf(out, ptr, "{%Q:%d,%Q:%Q,%Q:%d}",
                   "id", sqlite3_column_int(res, 0),
                   "name", sqlite3_column_text(res, 1),
                   "price", sqlite3_column_int(res, 2));
    }
    sqlite3_finalize(res);
}
