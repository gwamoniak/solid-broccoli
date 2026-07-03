qt_add_library(camera-core SHARED
    Album.cpp
    Album.h
    AlbumDAO.cpp
    AlbumDAO.h
    AlbumModel.cpp
    AlbumModel.h
    DatabaseManager.cpp
    DatabaseManager.h
    LoggerDAO.cpp
    LoggerDAO.h
    LoggerModel.cpp
    LoggerModel.h
    Movie.cpp
    Movie.h
    MovieDAO.cpp
    MovieDAO.h
    MovieModel.cpp
    MovieModel.h
    Picture.cpp
    Picture.h
    PictureDAO.cpp
    PictureDAO.h
    PictureModel.cpp
    PictureModel.h
    SessionDAO.cpp
    SessionDAO.h
    SessionModel.cpp
    SessionModel.h
    SessionSpectrumModel.cpp
    SessionSpectrumModel.h
    SpectrumDAO.cpp
    SpectrumDAO.h
    StorageLocations.cpp
    StorageLocations.h
    ThumbnailCache.cpp
    ThumbnailCache.h
    camera-core_global.h
)

target_link_libraries(camera-core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Sql
        logger-core
        spectro-core
)

target_compile_definitions(camera-core
    PRIVATE CAMERACORE_LIBRARY
)

target_include_directories(camera-core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
