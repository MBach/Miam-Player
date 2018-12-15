QT          += core gui multimedia network sql widgets
TEMPLATE    = lib

# Use system headers on Unix
win32 {
    include(chromaprint/chromaprint.pri)
}

DEFINES += MIAMACOUSTID_LIBRARY

SOURCES += \
    acoustid.cpp \
    matchingrecordswidget.cpp \
    mbrelease.cpp \
    qchromaprint.cpp \
    requestpool.cpp

HEADERS += \
    #chromaprint/debug.h \
    acoustid.h \
    matchingrecordswidget.h \
    mbrelease.h \
    qchromaprint.h \
    requestpool.h \
    miamacoustid_global.hpp

FORMS += matchingrecords.ui

CONFIG  +=  c++11

CONFIG(debug, debug|release) {
    win32 {
        LIBS += -L$$OUT_PWD/../core/debug/ -lmiam-core -lQtAV1 -L$$PWD/../../lib/ -L$$PWD/../../lib/debug/win-x64 -ltag -lchromaprint -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
    }
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG(release, debug|release) {
    win32 {
        LIBS += -L$$OUT_PWD/../core/release -lmiam-core -lQtAV1 -L$$PWD/../../lib/ -L$$PWD/../../lib/release/win-x64 -ltag -lchromaprint -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
    }
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}

TARGET      = miam-acoustid
unix {
    LIBS += -L$$OUT_PWD/../core/ -lmiam-core -lQtAV -ltag -lchromaprint
}
unix:!macx {
    target.path = /usr/lib$$LIB_SUFFIX/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

INCLUDEPATH += chromaprint/
DEPENDPATH += chromaprint

INCLUDEPATH += $$PWD/../core/ $$PWD/../core/3rdparty/ $$PWD/../core/3rdparty/QtAV/
DEPENDPATH += $$PWD/../core $$PWD/../core/3rdparty $$PWD/../core/3rdparty/QtAV
