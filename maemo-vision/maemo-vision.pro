TEMPLATE = app
TARGET = maemo-vision

SOURCES += main.cpp\
    MainWindow.cpp \
    CameraThread.cpp \
    OverlayWidget.cpp \
    RecognitionEngine.cpp \
    CameraParameters.cpp \
    ImageItem.cpp \
    UserDefaults.cpp \
    SoundPlayer.cpp \
    LEDBlinker.cpp \
    Viewfinder.cpp \
    ScrollArea.cpp \
    PanicHandler.cpp \
    ThumbnailView.cpp \
    SnapshotView.cpp \
    AppState.cpp

HEADERS  += MainWindow.h \
    CameraThread.h \
    OverlayWidget.h \
    RecognitionEngine.h \
    CameraParameters.h \
    ImageItem.h \
    UserDefaults.h \
    SoundPlayer.h \
    LEDBlinker.h \
    Viewfinder.h \
    ScrollArea.h \
    PanicHandler.h \
    ThumbnailView.h \
    SnapshotView.h \
    AppState.h \
    gourd.h

RESOURCES += \
    resources.qrc

INCLUDEPATH += /usr/local/include
INCLUDEPATH += ../../include
LIBS += -lXv -lpthread -lFCam -ltiff -lcv -lcxcore -lcvaux -lhighgui -lml -lflann -lopencv_lapack -llibjasper -lzlib -lpulse-simple
LIBS += -L/usr/local/lib
LIBS += -L../../lib

QT += core gui network opengl
QMAKE_CXXFLAGS += -DSINGLE_SENSOR -DSENSOR_NO=1 -mfpu=neon -mfloat-abi=softfp
