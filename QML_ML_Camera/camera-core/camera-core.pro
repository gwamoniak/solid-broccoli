QT       += sql
QT       -= gui

TARGET = camera-core
TEMPLATE = lib
CONFIG += lib c++17

DEFINES += CAMERACORE_LIBRARY

SOURCES += \
    AlbumModel.cpp \
    DatabaseManager.cpp \
    Album.cpp \
    Picture.cpp \
    PictureModel.cpp \
    Movie.cpp \
    MovieModel.cpp \
    PictureDAO.cpp \
    MovieDAO.cpp \
    AlbumDAO.cpp \
    LoggerDAO.cpp \
    LoggerModel.cpp \
    MovieAlbumDAO.cpp \
    MovieAlbumModel.cpp

HEADERS += \
    camera-core_global.h \
    AlbumModel.h \
    DatabaseManager.h \
    Album.h \
    Picture.h \
    PictureModel.h \
    Movie.h \
    MovieModel.h \
    PictureDAO.h \
    MovieDAO.h \
    AlbumDAO.h \
    LoggerDAO.h \
    LoggerModel.h \
    MovieAlbumDAO.h \
    MovieAlbumModel.h


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../logger-core/release/ -llogger-core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../logger-core/debug/ -llogger-core
else:unix:!macx: LIBS += -L$$OUT_PWD/../logger-core/ -llogger-core

INCLUDEPATH += $$PWD/../logger-core
DEPENDPATH += $$PWD/../logger-core
