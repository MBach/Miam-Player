QT += sql multimedia widgets

TEMPLATE = lib

DEFINES += MIAMUNIQUELIBRARY_LIBRARY

FORMS += \
    uniquelibrary.ui

HEADERS += \
    uniquelibrary.h \
    uniquelibraryitemdelegate.h \
    uniquelibraryitemmodel.h \
    miamuniquelibrary_global.hpp \
    uniquelibraryfilterproxymodel.h \
    coveritem.h \
    tableview.h

SOURCES += \
    uniquelibrary.cpp \
    uniquelibraryitemdelegate.cpp \
    uniquelibraryitemmodel.cpp \
    uniquelibraryfilterproxymodel.cpp \
    coveritem.cpp \
    tableview.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/debug/ -lMiamLibrary
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG += c++11
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/release/ -lMiamLibrary
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
win32 {
    TARGET = MiamUniqueLibrary
}
unix {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$OUT_PWD/../MiamCore/ -lmiam-core -L$$OUT_PWD/../MiamLibrary/ -lmiam-library
    TARGET = miam-uniquelibrary
}
unix:!macx {
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

INCLUDEPATH += $$PWD/../MiamCore/ $$PWD/../MiamLibrary/
DEPENDPATH += $$PWD/../MiamCore $$PWD/../MiamLibrary/

RESOURCES += uniqueLibrary.qrc

TRANSLATIONS = translations/uniqueLibrary_ar.ts \
    translations/uniqueLibrary_cs.ts \
    translations/uniqueLibrary_de.ts \
    translations/uniqueLibrary_en.ts \
    translations/uniqueLibrary_es.ts \
    translations/uniqueLibrary_fr.ts \
    translations/uniqueLibrary_in.ts \
    translations/uniqueLibrary_it.ts \
    translations/uniqueLibrary_ja.ts \
    translations/uniqueLibrary_kr.ts \
    translations/uniqueLibrary_pt.ts \
    translations/uniqueLibrary_ru.ts \
    translations/uniqueLibrary_th.ts \
    translations/uniqueLibrary_vn.ts \
    translations/uniqueLibrary_zh.ts
