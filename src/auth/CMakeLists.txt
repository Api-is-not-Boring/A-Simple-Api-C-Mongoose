add_library(auth STATIC auth.c bcrypt.c)

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

# Fetch Crypt Blowfish Repository ==================================================
include(FetchContent)
FetchContent_Declare(
        crypt_blowfish
        GIT_REPOSITORY https://github.com/openwall/crypt_blowfish.git
        GIT_TAG Owl-3_1-release
)

# Clone Repository ================================================================
FetchContent_MakeAvailable(crypt_blowfish)
add_library(crypt_blowfish INTERFACE)
target_sources(crypt_blowfish INTERFACE
        ${crypt_blowfish_SOURCE_DIR}/crypt_blowfish.c
        ${crypt_blowfish_SOURCE_DIR}/crypt_gensalt.c
        ${crypt_blowfish_SOURCE_DIR}/wrapper.c
        )
target_include_directories(crypt_blowfish INTERFACE ${crypt_blowfish_SOURCE_DIR})

target_link_libraries(auth
        PRIVATE l8w8jwt ${SQLite3_LIBRARIES} mongoose crypt_blowfish
        )

target_include_directories(auth
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/includes
        PRIVATE ${CMAKE_BINARY_DIR}
        )