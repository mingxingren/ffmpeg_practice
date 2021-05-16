QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

FFMPEG_PATH = D:/third_lib/ffmpeg/4.4
SDL_PATH = D:/third_lib/SDL/2.0.14

INCLUDEPATH += $$FFMPEG_PATH/include
INCLUDEPATH += $$SDL_PATH/x86_64-w64-mingw32/include

LIBS += $$FFMPEG_PATH/lib/avcodec.lib
LIBS += $$FFMPEG_PATH/lib/avdevice.lib
LIBS += $$FFMPEG_PATH/lib/avfilter.lib
LIBS += $$FFMPEG_PATH/lib/avformat.lib
LIBS += $$FFMPEG_PATH/lib/avutil.lib
LIBS += $$FFMPEG_PATH/lib/postproc.lib
LIBS += $$FFMPEG_PATH/lib/swresample.lib
LIBS += $$FFMPEG_PATH/lib/swscale.lib
LIBS += $$SDL_PATH/x86_64-w64-mingw32/lib/libSDL2.dll.a

SOURCES += \
    audiodecodethd.cpp \
    main.cpp \
    mainwindow.cpp \
    packqueue.cpp \
    playerthread.cpp \
    videowidget.cpp

HEADERS += \
    audiodecodethd.h \
    mainwindow.h \
    packetinfo.h \
    packqueue.h \
    playerthread.h \
    videowidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
