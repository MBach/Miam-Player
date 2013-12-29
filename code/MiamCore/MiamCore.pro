QT       += widgets multimedia sql

TagLibDirectory = $$PWD/3rdparty/taglib
INCLUDEPATH += $$PWD
INCLUDEPATH += $$TagLibDirectory
DEPENDPATH += $$TagLibDirectory

DEFINES += MIAM_PLUGIN

TARGET = MiamCore
TEMPLATE = lib

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/debug/ -ltag
    unix: LIBS += -L$$OUT_PWD -ltag
}
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/release/ -ltag
    unix: LIBS += -L$$OUT_PWD -ltag
}
win32: CONFIG += dll
unix: QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    model/librarysqlmodel.cpp \
    cover.cpp \
    filehelper.cpp \
    mediabutton.cpp \
    mediaplayer.cpp \
    libraryfilterlineedit.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp

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
    model/librarysqlmodel.h \
    mediabutton.h

unix:!symbian {
    target.path = /usr/lib
    INSTALLS += target
}
