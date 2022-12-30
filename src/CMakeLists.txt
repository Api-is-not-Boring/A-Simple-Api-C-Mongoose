add_executable(A_Simple_Api_C_Mongoose main.c)

add_subdirectory(models)
add_subdirectory(auth)
add_subdirectory(router)

target_link_libraries(A_Simple_Api_C_Mongoose PRIVATE mongoose models auth router)
