# Unit tests (Qt Test + CTest). Built when BUILD_TESTING is ON (the default,
# enabled by include(CTest) in the root list file). Run from the build dir with:
#     ctest --output-on-failure
find_package(Qt6 REQUIRED COMPONENTS Test)

# Declare a guiless Qt Test executable over camera-core and register it as a
# CTest case. Extra sources/args follow the name.
function(add_camera_test name)
    qt_add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Gui
        Qt6::Sql
        camera-core
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_camera_test(tst_storagelocations tst_storagelocations.cpp)
add_camera_test(tst_databasemanager tst_databasemanager.cpp)
add_camera_test(tst_albummodel      tst_albummodel.cpp)
add_camera_test(tst_picturemodel    tst_picturemodel.cpp)

# PluginManager lives in the app target, so compile its source straight into the
# test and point it at the freshly built grayscale plugin via a define.
add_camera_test(tst_pluginmanager
    tst_pluginmanager.cpp
    ${CMAKE_SOURCE_DIR}/app/PluginManager.cpp
)
target_include_directories(tst_pluginmanager PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_pluginmanager PRIVATE processor-api)
add_dependencies(tst_pluginmanager grayscale-processor)
target_compile_definitions(tst_pluginmanager PRIVATE
    GRAYSCALE_PLUGIN_DIR="$<TARGET_FILE_DIR:grayscale-processor>"
)

# The grayscale processor is a plugin (MODULE), so compile its source directly
# into this test to exercise process() without QPluginLoader.
add_camera_test(tst_frameprocessor_grayscale
    tst_frameprocessor_grayscale.cpp
    ${CMAKE_SOURCE_DIR}/plugins/grayscale/GrayscaleProcessor.cpp
)
target_include_directories(tst_frameprocessor_grayscale PRIVATE
    ${CMAKE_SOURCE_DIR}/plugins/grayscale
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_frameprocessor_grayscale PRIVATE processor-api)
