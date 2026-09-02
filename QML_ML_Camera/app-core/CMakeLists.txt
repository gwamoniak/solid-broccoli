# Application-domain logic shared by the executable and its unit tests,
# including the multimedia and scene-graph adapters exercised by smoke tests.
add_library(app-core STATIC
    ${CMAKE_SOURCE_DIR}/app/AppContext.cpp
    ${CMAKE_SOURCE_DIR}/app/AppContext.h
    ${CMAKE_SOURCE_DIR}/app/AppNotifier.cpp
    ${CMAKE_SOURCE_DIR}/app/AppNotifier.h
    ${CMAKE_SOURCE_DIR}/app/CameraService.cpp
    ${CMAKE_SOURCE_DIR}/app/CameraService.h
    ${CMAKE_SOURCE_DIR}/app/CaptureCoordinator.cpp
    ${CMAKE_SOURCE_DIR}/app/CaptureCoordinator.h
    ${CMAKE_SOURCE_DIR}/app/DetectionModel.cpp
    ${CMAKE_SOURCE_DIR}/app/DetectionModel.h
    ${CMAKE_SOURCE_DIR}/app/GeigerService.cpp
    ${CMAKE_SOURCE_DIR}/app/GeigerService.h
    ${CMAKE_SOURCE_DIR}/app/FrameProcessingWorker.cpp
    ${CMAKE_SOURCE_DIR}/app/FrameProcessingWorker.h
    ${CMAKE_SOURCE_DIR}/app/HelpContent.cpp
    ${CMAKE_SOURCE_DIR}/app/HelpContent.h
    ${CMAKE_SOURCE_DIR}/app/PeakListModel.cpp
    ${CMAKE_SOURCE_DIR}/app/PeakListModel.h
    ${CMAKE_SOURCE_DIR}/app/PluginManager.cpp
    ${CMAKE_SOURCE_DIR}/app/PluginManager.h
    ${CMAKE_SOURCE_DIR}/app/ReportContextBuilder.cpp
    ${CMAKE_SOURCE_DIR}/app/ReportContextBuilder.h
    ${CMAKE_SOURCE_DIR}/app/SpectrometerService.cpp
    ${CMAKE_SOURCE_DIR}/app/SpectrometerService.h
    ${CMAKE_SOURCE_DIR}/app/SpectrumOverlayProcessor.cpp
    ${CMAKE_SOURCE_DIR}/app/SpectrumOverlayProcessor.h
    ${CMAKE_SOURCE_DIR}/app/SpectrumView.cpp
    ${CMAKE_SOURCE_DIR}/app/SpectrumView.h
    ${CMAKE_SOURCE_DIR}/app/StripChartView.cpp
    ${CMAKE_SOURCE_DIR}/app/StripChartView.h
)

target_include_directories(app-core PUBLIC
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/processor-api
)

target_link_libraries(app-core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Quick
    camera-core
    logger-core
    processor-api
    spectro-core
    spectro-sim
)
