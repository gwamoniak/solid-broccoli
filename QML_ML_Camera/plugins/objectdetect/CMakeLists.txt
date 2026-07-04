# Object-detection plugin (Milestone 12). ONNX Runtime is a build-time
# OPTIONAL dependency of this plugin alone — the application never links it.
# Without the library the target is skipped with a status message and
# everything else builds and tests exactly as before (the bare-machine rule).
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
    PATH_SUFFIXES onnxruntime
    HINTS /opt/homebrew/include /usr/local/include $ENV{ORT_ROOT}/include
)
find_library(ONNXRUNTIME_LIBRARY onnxruntime
    HINTS /opt/homebrew/lib /usr/local/lib $ENV{ORT_ROOT}/lib
)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
    message(STATUS "objectdetect plugin: ONNX Runtime not found — target skipped "
                   "(brew install onnxruntime to enable)")
    return()
endif()

message(STATUS "objectdetect plugin: using ONNX Runtime at ${ONNXRUNTIME_LIBRARY}")

qt_add_plugin(objectdetect-processor
    SHARED
    CLASS_NAME ObjectDetectionProcessor
    ObjectDetectionProcessor.cpp
    ObjectDetectionProcessor.h
    VisionPost.cpp
    VisionPost.h
)

target_include_directories(objectdetect-processor PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})

target_link_libraries(objectdetect-processor
    PRIVATE
        Qt6::Core
        Qt6::Gui
        processor-api
        logger-core
        ${ONNXRUNTIME_LIBRARY}
)

# Emitted into app/vision/ — NOT app/plugins/ — so PluginManager's generic
# scan does not also list it: the detector is loaded explicitly by main.cpp
# and toggled from the Settings VISION section, like the spectrum overlay.
set_target_properties(objectdetect-processor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/app/vision"
)
