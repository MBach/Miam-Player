QT          += core gui multimedia network sql widgets

TEMPLATE    = lib

DEFINES     += MIAMCOVERFETCHER_LIBRARY

SOURCES += providers/amazonprovider.cpp \
    providers/musicbrainzprovider.cpp \
    providers/lastfmprovider.cpp \
    fetchdialog.cpp \
    browseimagewidget.cpp \
    coverfetcher.cpp

HEADERS += providers/amazonprovider.h \
    providers/coverartprovider.h \
    providers/lastfmprovider.h \
    providers/musicbrainzprovider.h \
    fetchdialog.h \
    browseimagewidget.h \
    miamcoverfetcher_global.hpp \
    coverfetcher.h

FORMS += fetchdialog.ui

CONFIG      += c++11
CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/debug/ -lmiam-core
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/release/ -lmiam-core
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}

TARGET      = miam-coverfetcher

!macx {
    LIBS += -L$$OUT_PWD/../core/ -lmiam-core
    target.path = /usr/lib$$LIB_SUFFIX/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

INCLUDEPATH += $$PWD/../core/
DEPENDPATH += $$PWD/../core
