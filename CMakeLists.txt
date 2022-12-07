cmake_minimum_required(VERSION 3.24.2)
project(A_Simple_Api_C_Mongoose C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Fetch Mongooses Repository and add to project ===================================
include(FetchContent)
FetchContent_Declare(
        mongoose
        GIT_REPOSITORY https://github.com/cesanta/mongoose.git
)
FetchContent_MakeAvailable(mongoose)
FetchContent_GetProperties(mongoose)
add_library(mongoose INTERFACE)
target_sources(mongoose INTERFACE ${mongoose_SOURCE_DIR}/mongoose.c)
target_include_directories(mongoose INTERFACE ${mongoose_SOURCE_DIR})

# Add the source directory ========================================================
add_subdirectory(src)
