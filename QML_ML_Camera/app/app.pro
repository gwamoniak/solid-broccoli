TEMPLATE = app
QT += qml quick sql svg
QT += multimedia
CONFIG += c++17




SOURCES += \
        main.cpp \
    CameraProcessor.cpp \
    PictureProvider.cpp

RESOURCES += qml.qrc
CONFIG+=qml_debug

HEADERS += \
    CameraProcessor.h \
    PictureProvider.h


VERSION = 1.0


DEFINES += APP_VERSION=\\\"$$VERSION\\\"
#OpenCV
win32: {
    INCLUDEPATH += "E:\\coding\\opencv\\build\\include"
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../coding/opencv/build/x64/vc15/lib/ -lopencv_world440
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../coding/opencv/build/x64/vc15/lib/ -lopencv_world440d
    INCLUDEPATH += $$PWD/../../../../../coding/opencv/build/x64/vc15
    DEPENDPATH += $$PWD/../../../../../coding/opencv/build/x64/vc15
}

#set up OpenCV fo linux
unix:!macx:{
    LIBS += -L/usr/local/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_ml \
    -lopencv_video \
    -lopencv_features2d \
    -lopencv_calib3d \
    -lopencv_objdetect \
    -lopencv_flann

    INCLUDEPATH +=/usr/local/include/opencv2
}


#logger-corea and camera-core set up

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../camera-core/release/ -lcamera-core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../camera-core/debug/ -lcamera-core
else:unix:!macx: LIBS += -L$$OUT_PWD/../camera-core/ -lcamera-core

INCLUDEPATH += $$PWD/../camera-core
DEPENDPATH += $$PWD/../camera-core

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../logger-core/release/ -llogger-core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../logger-core/debug/ -llogger-core
else:unix:!macx: LIBS += -L$$OUT_PWD/../logger-core/ -llogger-core

INCLUDEPATH += $$PWD/../logger-core
DEPENDPATH += $$PWD/../logger-core