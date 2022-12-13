add_executable(A_Simple_Api_C_Mongoose main.c)

target_sources(A_Simple_Api_C_Mongoose PRIVATE ${MONGOOSE_INCLUDE_DIR}/mongoose.c)
target_include_directories(A_Simple_Api_C_Mongoose PRIVATE ${MONGOOSE_INCLUDE_DIR})

add_subdirectory(models)
add_subdirectory(auth)
add_subdirectory(router)

target_link_libraries(A_Simple_Api_C_Mongoose PRIVATE models auth router)
