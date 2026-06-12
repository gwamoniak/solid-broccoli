# Header-only contract shared by the application (which loads plugins) and by
# every frame-processor plugin (which implements it). No compiled sources.
add_library(processor-api INTERFACE)

target_include_directories(processor-api
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(processor-api
    INTERFACE
        Qt6::Core
        Qt6::Gui
)
