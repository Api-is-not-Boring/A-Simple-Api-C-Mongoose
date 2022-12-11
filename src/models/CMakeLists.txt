# Add Sqlite3 library =============================================================
include(FindSQLite3)
if(SQLITE3_FOUND)
    message(STATUS "SQLite3 found: v${SQLite3_VERSION}")
else()
    message(FATAL_ERROR "SQLite3 not found")
endif()

add_library(model STATIC model.c)

target_link_libraries(model PUBLIC ${SQLite3_LIBRARIES} mongoose)

target_include_directories(model
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes
        )