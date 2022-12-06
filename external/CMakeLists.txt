add_library(mongoose "")

target_sources(mongoose
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/mongoose.c
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/mongoose.h
        )
target_include_directories(mongoose PUBLIC ${CMAKE_CURRENT_LIST_DIR})
