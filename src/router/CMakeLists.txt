add_library(router)

target_sources(router
        PRIVATE router.c
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/router.h
        )

target_link_libraries(router PUBLIC mongoose)
target_include_directories(router PUBLIC ${CMAKE_CURRENT_LIST_DIR})