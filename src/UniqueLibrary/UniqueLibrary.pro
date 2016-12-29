QT += sql multimedia widgets

TEMPLATE = lib

DEFINES += MIAMUNIQUELIBRARY_LIBRARY

FORMS += uniquelibrary.ui

HEADERS += coveritem.h \
    tableview.h \
    miamuniquelibrary_global.hpp \
    uniquelibrary.h \
    uniquelibraryitemdelegate.h \
    uniquelibraryitemmodel.h \
    uniquelibraryfilterproxymodel.h \
    uniquelibrarymediaplayercontrol.h

SOURCES += coveritem.cpp \
    tableview.cpp \
    uniquelibrary.cpp \
    uniquelibraryitemdelegate.cpp \
    uniquelibraryitemmodel.cpp \
    uniquelibraryfilterproxymodel.cpp \
    uniquelibrarymediaplayercontrol.cpp

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/debug/ -lmiam-core -L$$OUT_PWD/../library/debug/ -lmiam-library
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG += c++11
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../core/release/ -lmiam-core -L$$OUT_PWD/../library/release/ -lmiam-library
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}

TARGET = miam-uniquelibrary

unix {
    LIBS += -L$$OUT_PWD/../core/ -lmiam-core -L$$OUT_PWD/../library/ -lmiam-library
}
unix:!macx {
    target.path = /usr/lib$$LIB_SUFFIX/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

INCLUDEPATH += $$PWD/../core/ $$PWD/../library/
DEPENDPATH += $$PWD/../core $$PWD/../library/

RESOURCES += uniquelibrary.qrc

TRANSLATIONS = translations/uniquelibrary_ar.ts \
    translations/uniquelibrary_cs.ts \
    translations/uniquelibrary_de.ts \
    translations/uniquelibrary_el.ts \
    translations/uniquelibrary_en.ts \
    translations/uniquelibrary_es.ts \
    translations/uniquelibrary_fr.ts \
    translations/uniquelibrary_in.ts \
    translations/uniquelibrary_it.ts \
    translations/uniquelibrary_ja.ts \
    translations/uniquelibrary_kr.ts \
    translations/uniquelibrary_pt.ts \
    translations/uniquelibrary_ru.ts \
    translations/uniquelibrary_th.ts \
    translations/uniquelibrary_vn.ts \
    translations/uniquelibrary_zh.ts
