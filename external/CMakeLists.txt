add_library(mongoose "")

target_sources(mongoose
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/mongoose/mongoose.c
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/mongoose/mongoose.h
        )
target_include_directories(mongoose PUBLIC ${CMAKE_CURRENT_LIST_DIR})
