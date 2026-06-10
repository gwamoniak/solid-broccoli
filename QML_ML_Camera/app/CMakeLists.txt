qt_add_executable(QML_ML_Camera_App
    MACOSX_BUNDLE
    main.cpp
    CameraService.cpp
    CameraService.h
    PictureProvider.cpp
    PictureProvider.h
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
        PageTheme.qml
        PicturePage.qml
        SettingSwitcher.qml
        Style.qml
        ToolBarTheme.qml
        main.qml
        qmldir
        images/png/add_album_gallery.png
        images/png/add_photo.png
        images/png/album_gallery.png
        images/png/back.png
        images/png/camera.png
        images/png/cogwheel.png
        images/png/delete_album_gallery.png
        images/png/delete_photo.png
        images/png/forward_player.png
        images/png/help.png
        images/png/home_button.png
        images/png/open_log.png
        images/png/photo.png
        images/png/quit.png
        images/png/record.png
        images/png/rename_album_gallery.png
        images/png/rename_photo.png
        images/png/save_photo.png
)

target_link_libraries(QML_ML_Camera_App
    PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::Sql
        Qt6::Svg
        Qt6::Multimedia
        camera-core
        logger-core
)

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
