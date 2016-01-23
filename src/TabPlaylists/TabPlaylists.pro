QT += multimedia sql widgets

TEMPLATE = lib

DEFINES += MIAMTABPLAYLISTS_LIBRARY

SOURCES += dialogs/closeplaylistpopup.cpp \
    cornerwidget.cpp \
    playbackmodewidget.cpp \
    playbackmodewidgetfactory.cpp \
    playlist.cpp \
    playlistheaderview.cpp \
    playlistmanager.cpp \
    playlistmodel.cpp \
    tabbar.cpp \
    tabplaylist.cpp \
    playlistitemdelegate.cpp \
    stareditor.cpp

HEADERS += dialogs/closeplaylistpopup.h \
    cornerwidget.h \
    playbackmodewidget.h \
    playbackmodewidgetfactory.h \
    playlist.h \
    playlistheaderview.h \
    playlistitemdelegate.h \
    playlistmanager.h \
    playlistmodel.h \
    tabbar.h \
    tabplaylist.h \
    stareditor.h \
    miamtabPlaylists_global.hpp \
    abstractviewplaylists.h

FORMS += closeplaylistpopup.ui \
    playlistdialog.ui

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../Core/debug/ -lCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG += c++11
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../Core/release/ -lCore
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
win32 {
    TARGET = TabPlaylists
}
unix {
    LIBS += -L$$OUT_PWD/../Core/ -lmiam-core
    TARGET = miam-tabplaylists
}
unix:!macx {
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

INCLUDEPATH += $$PWD/../Core/
DEPENDPATH += $$PWD/../Core

TRANSLATIONS = translations/TabPlaylists_ar.ts \
    translations/TabPlaylists_cs.ts \
    translations/TabPlaylists_de.ts \
    translations/TabPlaylists_en.ts \
    translations/TabPlaylists_es.ts \
    translations/TabPlaylists_fr.ts \
    translations/TabPlaylists_in.ts \
    translations/TabPlaylists_it.ts \
    translations/TabPlaylists_ja.ts \
    translations/TabPlaylists_kr.ts \
    translations/TabPlaylists_pt.ts \
    translations/TabPlaylists_ru.ts \
    translations/TabPlaylists_th.ts \
    translations/TabPlaylists_vn.ts \
    translations/TabPlaylists_zh.ts
