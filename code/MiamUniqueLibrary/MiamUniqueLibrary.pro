QT += sql widgets

TEMPLATE = lib

DEFINES += MIAMUNIQUELIBRARY_LIBRARY

FORMS += templateAlbum.ui \
    uniquelibrary.ui

HEADERS += albumform.h \
    miamuniquelibrary_global.h \
    uniquelibrary.h

SOURCES += albumform.cpp \
    uniquelibrary.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}

win32: DEFINES += Q_COMPILER_INITIALIZER_LISTS

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -lMiamCore
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}
win32 {
    TARGET = MiamUniqueLibrary
}
unix {
    LIBS += -L$$OUT_PWD/../MiamCore/ -lmiam-core
    TARGET = miam-uniquelibrary
}
unix:!macx {
    target.path = /usr/lib/
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../MiamCore/
DEPENDPATH += $$PWD/../MiamCore
