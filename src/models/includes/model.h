#ifndef A_SIMPLE_API_C_MONGOOSE_MODEL_H
#define A_SIMPLE_API_C_MONGOOSE_MODEL_H

#include <sqlite3.h>
#include "mongoose.h"

void db_init(sqlite3 **db);

void db_close(sqlite3 *db);

void db_count_all_cars(mg_pfn_t out, void *ptr, va_list *ap);

void db_get_all_cars(mg_pfn_t out, void *ptr, va_list *ap);

void db_get_car_by_id(mg_pfn_t out, void *ptr, va_list *ap);

bool db_car_is_existed(sqlite3 *db, int id);

bool db_create_car(sqlite3 *db, const char *name, int price);

bool db_update_car(sqlite3 *db, int id, const char *name, int price);

bool db_delete_car(sqlite3 *db, int id);

#endif //A_SIMPLE_API_C_MONGOOSE_MODEL_H
