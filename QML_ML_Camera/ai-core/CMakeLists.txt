# On-device report generator (Milestone 13). llama.cpp is fetched at
# configure time, pinned to a tag; the whole target is optional — configure
# with -DAI_ANALYST=OFF (or without network on first configure) and the app
# builds without it, per the bare-machine rule.
include(FetchContent)

set(LLAMA_CURL OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)

FetchContent_Declare(llama
    GIT_REPOSITORY https://github.com/ggml-org/llama.cpp
    GIT_TAG        b6100
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(llama)

qt_add_library(ai-core STATIC
    AiAnalyst.cpp
    AiAnalyst.h
)

target_link_libraries(ai-core
    PUBLIC
        Qt6::Core
        llama
    PRIVATE
        logger-core
)

target_include_directories(ai-core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
