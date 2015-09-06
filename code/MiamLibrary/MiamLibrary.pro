QT += sql multimedia widgets

TEMPLATE = lib

DEFINES += MIAMLIBRARY_LIBRARY

SOURCES += \
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
    separatoritem.cpp \
    scrollbar.cpp \
    trackitem.cpp \
    yearitem.cpp \
    deprecated/circleprogressbar.cpp

HEADERS += \
    albumitem.h \
    artistitem.h \
    changehierarchybutton.h \
    discitem.h \
    extendedtabbar.h \
    extendedtabwidget.h \
    libraryfilterproxymodel.h \
    libraryheader.h \
    libraryitemdelegate.h \
    libraryitemmodel.h \
    libraryfilterlineedit.h \
    libraryorderdialog.h \
    libraryscrollbar.h \
    librarytreeview.h \
    separatoritem.h \
    scrollbar.h \
    trackitem.h \
    yearitem.h \
    miamlibrary_global.h \
    deprecated/circleprogressbar.h

FORMS += \
    libraryorderdialog.ui

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

win32: DEFINES += Q_COMPILER_INITIALIZER_LISTS

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

INCLUDEPATH += $$PWD/../MiamCore/
DEPENDPATH += $$PWD/../MiamCore
