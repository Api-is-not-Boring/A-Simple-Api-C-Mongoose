add_library(auth STATIC auth.c)

# Fetch l8w8jwt Repository ========================================================
include(FetchContent)
FetchContent_Declare(
        l8w8jwt
        GIT_REPOSITORY https://github.com/GlitchedPolygons/l8w8jwt.git
        GIT_SUBMODULES_RECURSE TRUE
        GIT_PROGRESS TRUE
)

# Clone Repository ================================================================
FetchContent_MakeAvailable(l8w8jwt)

target_link_libraries(auth
        PRIVATE l8w8jwt ${SQLite3_LIBRARIES}
        )

target_include_directories(auth
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes ${MONGOOSE_INCLUDE_DIR}
        PRIVATE ${CMAKE_BINARY_DIR}
        )