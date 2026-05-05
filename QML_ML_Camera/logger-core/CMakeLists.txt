qt_add_library(logger-core SHARED
    logger.cpp
    logger.h
    logger-core_global.h
    loggingcategories.cpp
    loggingcategories.h
)

target_link_libraries(logger-core
    PUBLIC Qt6::Core
)

target_compile_definitions(logger-core
    PRIVATE LOGGERCORE_LIBRARY
)

target_include_directories(logger-core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
