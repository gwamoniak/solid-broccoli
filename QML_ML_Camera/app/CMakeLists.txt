qt_add_executable(QML_ML_Camera_App
    MACOSX_BUNDLE
    main.cpp
    AppSettings.cpp
    AppSettings.h
    CameraService.cpp
    CameraService.h
    CaptureCoordinator.cpp
    CaptureCoordinator.h
    FrameProcessingWorker.cpp
    FrameProcessingWorker.h
    PluginManager.cpp
    PluginManager.h
    PictureProvider.cpp
    PictureProvider.h
    SpectrometerService.cpp
    SpectrometerService.h
)

qt_add_resources(QML_ML_Camera_App app_resources
    PREFIX /
    FILES
        AlbumListPage.qml
        AlbumPage.qml
        CameraPage.qml
        InputDialog.qml
        LoggerPage.qml
        MainPage.qml
        MovieAlbumPage.qml
        MoviePage.qml
        NavPage.qml
        PageTheme.qml
        PicturePage.qml
        SettingsPage.qml
        SettingSwitcher.qml
        Style.qml
        Theme.qml
        ToolBarTheme.qml
        main.qml
        qmldir
        images/svg/add-album.svg
        images/svg/add-photo.svg
        images/svg/back.svg
        images/svg/camera.svg
        images/svg/delete.svg
        images/svg/gallery.svg
        images/svg/home.svg
        images/svg/log.svg
        images/svg/movie.svg
        images/svg/quit.svg
        images/svg/record.svg
        images/svg/rename.svg
        images/svg/save.svg
        images/svg/settings.svg
)

target_link_libraries(QML_ML_Camera_App
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Sql
        Qt6::Svg
        Qt6::Multimedia
        camera-core
        logger-core
        processor-api
        spectro-core
        spectro-sim
)

# Build the proof plugin alongside the app so it lands in app/plugins/ and is
# discovered at runtime, even though the app does not link it.
add_dependencies(QML_ML_Camera_App grayscale-processor)

target_compile_definitions(QML_ML_Camera_App
    PRIVATE APP_VERSION="${PROJECT_VERSION}"
)

target_include_directories(QML_ML_Camera_App
    PRIVATE
        ${CMAKE_SOURCE_DIR}/camera-core
        ${CMAKE_SOURCE_DIR}/logger-core
)

set_target_properties(QML_ML_Camera_App PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
)
