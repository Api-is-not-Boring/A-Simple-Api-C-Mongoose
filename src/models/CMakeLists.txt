add_library(models STATIC model.c)

target_link_libraries(models PRIVATE mongoose ${SQLite3_LIBRARIES} auth)

target_include_directories(models
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes
        )