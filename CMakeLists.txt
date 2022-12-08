cmake_minimum_required(VERSION 3.24)
project(A_Simple_Api_C_Mongoose C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Fetch Mongooses Repository ======================================================
include(FetchContent)
FetchContent_Declare(
        mongoose
        GIT_REPOSITORY https://github.com/cesanta/mongoose.git
)
FetchContent_MakeAvailable(mongoose)

# Add the Mongoose Library ========================================================
add_library(mongoose INTERFACE)
target_sources(mongoose INTERFACE ${mongoose_SOURCE_DIR}/mongoose.c)
target_include_directories(mongoose INTERFACE ${mongoose_SOURCE_DIR})

# Add the Source directory ========================================================
add_subdirectory(src)
