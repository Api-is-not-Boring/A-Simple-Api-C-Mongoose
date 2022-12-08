add_library(router STATIC router.c)
target_link_libraries(router PUBLIC mongoose)
target_include_directories(router PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes)