# Unit tests (Qt Test + CTest). Built when BUILD_TESTING is ON (the default,
# enabled by include(CTest) in the root list file). Run from the build dir with:
#     ctest --output-on-failure
find_package(Qt6 REQUIRED COMPONENTS Test)

find_program(QMLLINT_EXECUTABLE NAMES qmllint HINTS "${QT_HOST_PATH}/bin")
if(QMLLINT_EXECUTABLE)
    file(GLOB APP_QML_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/app/*.qml")
    add_test(NAME tst_qmllint
        COMMAND "${QMLLINT_EXECUTABLE}"
                -I "${CMAKE_SOURCE_DIR}/qmltypes"
                ${APP_QML_SOURCES})
endif()

add_test(NAME tst_qml_boot
    COMMAND $<TARGET_FILE:QML_ML_Camera_App> -platform offscreen)
set_tests_properties(tst_qml_boot PROPERTIES
    ENVIRONMENT "SOLIDBROCCOLI_QML_SMOKE=1")

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

# spectro-core tests: pure analysis math and the transport+codec device wiring.
add_camera_test(tst_spectroanalysis tst_spectroanalysis.cpp)
target_link_libraries(tst_spectroanalysis PRIVATE spectro-core)
add_camera_test(tst_codecdevice tst_codecdevice.cpp)
target_link_libraries(tst_codecdevice PRIVATE spectro-core)

# spectro-sim: analysis validated against simulator-injected ground truth.
add_camera_test(tst_spectrosim tst_spectrosim.cpp)
target_link_libraries(tst_spectrosim PRIVATE spectro-core spectro-sim)

# Application services are shared through app-core.
add_camera_test(tst_spectrometerservice
    tst_spectrometerservice.cpp
)
target_include_directories(tst_spectrometerservice PRIVATE ${CMAKE_SOURCE_DIR}/app)
target_link_libraries(tst_spectrometerservice PRIVATE app-core)

# Session persistence + export (Milestone 7).
add_camera_test(tst_sessiondao tst_sessiondao.cpp)
target_link_libraries(tst_sessiondao PRIVATE spectro-core spectro-sim)
add_camera_test(tst_spectrumexporter tst_spectrumexporter.cpp)
target_link_libraries(tst_spectrumexporter PRIVATE spectro-core spectro-sim)

# Bridge contract codec + simulated bridge byte path (Milestone 9).
add_camera_test(tst_as7265xcodec tst_as7265xcodec.cpp)
target_link_libraries(tst_as7265xcodec PRIVATE spectro-core spectro-sim)
add_camera_test(tst_bridgeframeparser tst_bridgeframeparser.cpp)
target_link_libraries(tst_bridgeframeparser PRIVATE spectro-core)
add_camera_test(tst_appnotifier tst_appnotifier.cpp)
target_link_libraries(tst_appnotifier PRIVATE app-core)
add_camera_test(tst_spectrumview tst_spectrumview.cpp)
target_link_libraries(tst_spectrumview PRIVATE app-core Qt6::Quick)

# Geiger modality (Milestone 10): Poisson ground truth, rolling CPM/dose,
# codec dispatch, bridge byte path, MeasurementDAO, and the service.
add_camera_test(tst_geiger
    tst_geiger.cpp
)
target_include_directories(tst_geiger PRIVATE ${CMAKE_SOURCE_DIR}/app)
target_link_libraries(tst_geiger PRIVATE app-core)

# Video overlay processor (Milestone 8).
add_camera_test(tst_spectrumoverlay
    tst_spectrumoverlay.cpp
)
target_include_directories(tst_spectrumoverlay PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_spectrumoverlay PRIVATE app-core)
add_camera_test(tst_albummodel      tst_albummodel.cpp)
add_camera_test(tst_picturemodel    tst_picturemodel.cpp)

# Point the shared PluginManager at the freshly built grayscale plugin.
add_camera_test(tst_pluginmanager
    tst_pluginmanager.cpp
)
target_include_directories(tst_pluginmanager PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_pluginmanager PRIVATE app-core)
add_dependencies(tst_pluginmanager grayscale-processor)
target_compile_definitions(tst_pluginmanager PRIVATE
    GRAYSCALE_PLUGIN_DIR="$<TARGET_FILE_DIR:grayscale-processor>"
)

# AI analyst grounding (Milestone 13): deterministic identification, report
# context, and report persistence — all model-free, run everywhere.
add_camera_test(tst_peakmatcher tst_peakmatcher.cpp)
target_link_libraries(tst_peakmatcher PRIVATE spectro-core spectro-sim)
add_camera_test(tst_reportcontext
    tst_reportcontext.cpp
)
target_include_directories(tst_reportcontext PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_reportcontext PRIVATE app-core)
add_camera_test(tst_reportdao tst_reportdao.cpp)
target_link_libraries(tst_reportdao PRIVATE spectro-core)

# Generation smoke test: only built with the AI target, and QSKIPs unless
# SOLIDBROCCOLI_TEST_GGUF points at a model file.
if(TARGET ai-core)
    add_camera_test(tst_aianalyst tst_aianalyst.cpp)
    target_link_libraries(tst_aianalyst PRIVATE ai-core)
endif()

# Vision pipeline v1.1 (Milestone 12): pure detector math (no ONNX Runtime,
# no model) and the structured-result channel into DetectionModel.
add_camera_test(tst_visionpost
    tst_visionpost.cpp
    ${CMAKE_SOURCE_DIR}/plugins/objectdetect/VisionPost.cpp
)
target_include_directories(tst_visionpost PRIVATE
    ${CMAKE_SOURCE_DIR}/plugins/objectdetect
)
add_camera_test(tst_pipelinemetadata
    tst_pipelinemetadata.cpp
)
target_include_directories(tst_pipelinemetadata PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)
target_link_libraries(tst_pipelinemetadata PRIVATE app-core)

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
