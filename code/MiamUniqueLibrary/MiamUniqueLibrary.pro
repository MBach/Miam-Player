QT += sql widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = MiamUniqueLibrary
TEMPLATE = lib

DEFINES += MIAMUNIQUELIBRARY_LIBRARY

FORMS += \
    uniquelibrary.ui \
    templateAlbum.ui

HEADERS += \
    uniquelibrary.h \
    miamuniquelibrary_global.h \
    flowlayout.h \
    albumform.h

SOURCES += \
    uniquelibrary.cpp \
    flowlayout.cpp \
    albumform.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -lMiamCore
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}
unix: LIBS += -L$$OUT_PWD/../MiamCore/ -lMiamCore

INCLUDEPATH += $$PWD/../MiamCore/
DEPENDPATH += $$PWD/../MiamCore
