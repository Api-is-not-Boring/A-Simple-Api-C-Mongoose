#ifndef A_SIMPLE_API_C_MONGOOSE_MODEL_H
#define A_SIMPLE_API_C_MONGOOSE_MODEL_H

#include <sqlite3.h>
#include "mongoose.h"

void db_init(sqlite3 **db);

void db_close(sqlite3 *db);

void db_get_all_cars(mg_pfn_t out, void *ptr, va_list *ap);

void db_get_car_by_id(mg_pfn_t out, void *ptr, va_list *ap);

#endif //A_SIMPLE_API_C_MONGOOSE_MODEL_H
