QT += sql multimedia widgets

TEMPLATE = lib

DEFINES += MIAMUNIQUELIBRARY_LIBRARY

FORMS += \
    uniquelibrary.ui

HEADERS += \
    miamuniquelibrary_global.h \
    uniquelibrary.h

SOURCES += \
    uniquelibrary.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/debug/ -lMiamLibrary
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}

win32: DEFINES += Q_COMPILER_INITIALIZER_LISTS

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/release/ -lMiamLibrary
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}
win32 {
    TARGET = MiamUniqueLibrary
}
unix {
    LIBS += -L$$OUT_PWD/../MiamCore/ -lmiam-core -L$$OUT_PWD/../MiamLibrary/ -lmiam-library
    TARGET = miam-uniquelibrary
    CONFIG += c++11
    QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
}

INCLUDEPATH += $$PWD/../MiamCore/ $$PWD/../MiamLibrary/
DEPENDPATH += $$PWD/../MiamCore $$PWD/../MiamLibrary/
