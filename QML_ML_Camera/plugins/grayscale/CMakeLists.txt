# MODULE = a runtime-loadable shared object (not linked at build time). It is
# emitted into <build>/app/plugins/ so the running application finds it next to
# the executable's plugins/ scan directory.
qt_add_plugin(grayscale-processor
    SHARED
    CLASS_NAME GrayscaleProcessor
    GrayscaleProcessor.cpp
    GrayscaleProcessor.h
)

target_link_libraries(grayscale-processor
    PRIVATE
        Qt6::Core
        Qt6::Gui
        processor-api
)

set_target_properties(grayscale-processor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/app/plugins"
)
