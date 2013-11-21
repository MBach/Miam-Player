#-------------------------------------------------
#
# Project created by QtCreator 2013-11-19T19:50:21
#
#-------------------------------------------------

QT       += widgets

QT       -= gui

TagLibDirectory = $$PWD/3rdparty/taglib
INCLUDEPATH += $$PWD
INCLUDEPATH += $$TagLibDirectory
DEPENDPATH += $$TagLibDirectory

TARGET = MiamCore
TEMPLATE = lib
#CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$PWD/debug/ -llibtag
    unix: LIBS += -L$$PWD/debug/ -ltag
}
CONFIG(release, debug|release) {
    win32: LIBS += -L$$PWD/release/ -llibtag
    unix: LIBS += -L$$PWD/release/ -ltag
}

SOURCES += \
    settings.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    filehelper.cpp \
    cover.cpp

HEADERS += \
    settings.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    filehelper.h \
    cover.h

unix:!symbian {
    target.path = /usr/lib
    INSTALLS += target
}
