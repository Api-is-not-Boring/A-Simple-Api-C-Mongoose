add_library(auth STATIC auth.c)

target_link_libraries(auth
        PUBLIC mongoose model
        PRIVATE l8w8jwt
        )

target_include_directories(auth
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes
        PRIVATE ${CMAKE_BINARY_DIR}
        )