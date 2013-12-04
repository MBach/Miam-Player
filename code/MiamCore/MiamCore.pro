#-------------------------------------------------
#
# Project created by QtCreator 2013-11-19T19:50:21
#
#-------------------------------------------------

QT       += widgets multimedia

QT       -= gui

TagLibDirectory = $$PWD/3rdparty/taglib
INCLUDEPATH += $$PWD
INCLUDEPATH += $$TagLibDirectory
DEPENDPATH += $$TagLibDirectory

DEFINES += MIAMCORE_LIBRARY

TARGET = MiamCore
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/debug/ -llibtag
    unix: LIBS += -L$$OUT_PWD -ltag
}
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/release/ -llibtag
    unix: LIBS += -L$$OUT_PWD -ltag
}

SOURCES += \
    settings.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    filehelper.cpp \
    cover.cpp \
    mediaplayer.cpp \
    libraryfilterlineedit.cpp \
    model/librarymodel.cpp \
    model/libraryitem.cpp \
    model/persistentitem.cpp \
    model/libraryitemdiscnumber.cpp

HEADERS += \
    settings.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    filehelper.h \
    cover.h \
    miamcore_global.h \
    mediaplayer.h \
    interfaces/basicplugininterface.h \
    interfaces/mediaplayerplugininterface.h \
    libraryfilterlineedit.h \
    model/librarymodel.h \
    model/libraryitem.h \
    model/libraryitemalbum.h \
    model/libraryitemartist.h \
    model/libraryitemletter.h \
    model/libraryitemtrack.h \
    model/persistentitem.h \
    model/libraryitemdiscnumber.h

unix:!symbian {
    target.path = /usr/lib
    INSTALLS += target
}
