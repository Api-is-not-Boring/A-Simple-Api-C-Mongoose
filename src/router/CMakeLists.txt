add_library(router STATIC router.c)

target_link_libraries(router PRIVATE models auth)

target_include_directories(router
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes ${MONGOOSE_INCLUDE_DIR}
        PRIVATE ${CMAKE_BINARY_DIR}
)