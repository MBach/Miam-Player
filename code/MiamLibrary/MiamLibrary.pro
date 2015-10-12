QT += sql multimedia widgets

TEMPLATE = lib

DEFINES += MIAMLIBRARY_LIBRARY

SOURCES += \
    deprecated/circleprogressbar.cpp \
    albumitem.cpp \
    artistitem.cpp \
    changehierarchybutton.cpp \
    discitem.cpp \
    extendedtabbar.cpp \
    libraryfilterlineedit.cpp \
    libraryfilterproxymodel.cpp \
    libraryheader.cpp \
    libraryitemdelegate.cpp \
    libraryitemmodel.cpp \
    libraryorderdialog.cpp \
    libraryscrollbar.cpp \
    librarytreeview.cpp \
    miamitemdelegate.cpp \
    miamitemmodel.cpp \
    separatoritem.cpp \
    scrollbar.cpp \
    trackitem.cpp \
    yearitem.cpp

HEADERS += \
    deprecated/circleprogressbar.h \
    albumitem.h \
    artistitem.h \
    changehierarchybutton.h \
    discitem.h \
    extendedtabbar.h \
    extendedtabwidget.h \
    libraryfilterlineedit.h \
    libraryfilterproxymodel.h \
    libraryheader.h \
    libraryitemdelegate.h \
    libraryitemmodel.h \
    libraryorderdialog.h \
    libraryscrollbar.h \
    librarytreeview.h \
    miamlibrary_global.h \
    miamitemdelegate.h \
    miamitemmodel.h \
    separatoritem.h \
    scrollbar.h \
    trackitem.h \
    yearitem.h

FORMS += \
    libraryorderdialog.ui

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG += c++11
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -lMiamCore
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
win32 {
    TARGET = MiamLibrary
}
unix {
    LIBS += -L$$OUT_PWD/../MiamCore/ -lmiam-core
    TARGET = miam-library
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

INCLUDEPATH += $$PWD/../MiamCore/
DEPENDPATH += $$PWD/../MiamCore
