QT          += core gui multimedia network sql widgets

TEMPLATE    = lib

DEFINES     += MIAMTAGEDITOR_LIBRARY

SOURCES += albumcover.cpp \
    tagconverter.cpp \
    tageditor.cpp \
    tageditortablewidget.cpp \
    taglineedit.cpp \
    tagbutton.cpp

HEADERS += miamtageditor_global.hpp \
    albumcover.h \
    tagconverter.h \
    tageditor.h \
    tageditortablewidget.h \
    taglineedit.h \
    tagbutton.h

FORMS += tagconverter.ui \
    tageditor.ui

CONFIG      += c++11
CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/debug/ -lmiam-core -L$$OUT_PWD/../acoustid/debug/ -lmiam-acoustid
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/release/ -lmiam-core -L$$OUT_PWD/../acoustid/release/ -lmiam-acoustid
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}

TARGET      = miam-tageditor

!macx {
    LIBS += -L$$OUT_PWD/../core/ -lmiam-core -L$$OUT_PWD/../acoustid/ -lmiam-acoustid
    target.path = /usr/lib$$LIB_SUFFIX/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

3rdpartyDir  = $$PWD/../core/3rdparty
INCLUDEPATH += $$3rdpartyDir
DEPENDPATH += $$3rdpartyDir

INCLUDEPATH += $$PWD/../core/ $$PWD/../acoustid/
DEPENDPATH += $$PWD/../core $$PWD/../acoustid

RESOURCES += tageditor.qrc
