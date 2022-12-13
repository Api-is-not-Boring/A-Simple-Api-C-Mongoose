add_library(models STATIC model.c)

target_link_libraries(models PUBLIC ${SQLite3_LIBRARIES})

target_include_directories(models
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes ${MONGOOSE_INCLUDE_DIR}
        )