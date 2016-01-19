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
    miamtabplaylists_global.hpp

FORMS += closeplaylistpopup.ui

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
    TARGET = MiamTabPlaylists
}
unix {
    LIBS += -L$$OUT_PWD/../MiamCore/ -lmiam-core
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

INCLUDEPATH += $$PWD/../MiamCore/
DEPENDPATH += $$PWD/../MiamCore

TRANSLATIONS = translations/MiamTabPlaylists_ar.ts \
    translations/MiamTabPlaylists_cs.ts \
    translations/MiamTabPlaylists_de.ts \
    translations/MiamTabPlaylists_en.ts \
    translations/MiamTabPlaylists_es.ts \
    translations/MiamTabPlaylists_fr.ts \
    translations/MiamTabPlaylists_in.ts \
    translations/MiamTabPlaylists_it.ts \
    translations/MiamTabPlaylists_ja.ts \
    translations/MiamTabPlaylists_kr.ts \
    translations/MiamTabPlaylists_pt.ts \
    translations/MiamTabPlaylists_ru.ts \
    translations/MiamTabPlaylists_th.ts \
    translations/MiamTabPlaylists_vn.ts \
    translations/MiamTabPlaylists_zh.ts
