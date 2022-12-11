cmake_minimum_required(VERSION 3.24)

# Project settings ================================================================
project("A Simple Api C Mongoose"
        VERSION 1.0.0
        LANGUAGES C
        DESCRIPTION "A Simple Api service build with C and Mongoose framework"
        HOMEPAGE_URL "https://github.com/Api-is-not-Boring/A-Simple-Api-C-Mongoose"
        )

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FindGit)
if(GIT_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    set(GIT_HASH "unknown")
endif()

message(STATUS "Git hash is ${GIT_HASH}")

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
configure_file(project.h.in project.h @ONLY)
