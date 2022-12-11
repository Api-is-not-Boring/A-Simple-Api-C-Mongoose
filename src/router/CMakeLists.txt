add_library(router STATIC router.c)

target_link_libraries(router PUBLIC mongoose model)

target_include_directories(router
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes ${CMAKE_BINARY_DIR}
)