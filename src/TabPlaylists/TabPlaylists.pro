QT += multimedia sql widgets

TEMPLATE = lib

DEFINES += MIAMTABPLAYLISTS_LIBRARY

SOURCES += dialogs/closeplaylistpopup.cpp \
    dialogs/playlistdialog.cpp \
    dialogs/searchdialog.cpp \
    filesystem/addressbar.cpp \
    filesystem/addressbarbutton.cpp \
    filesystem/addressbardirectorylist.cpp \
    filesystem/addressbarlineedit.cpp \
    filesystem/addressbarmenu.cpp \
    filesystem/filesystemtreeview.cpp \
    mediabuttons/playbackmodebutton.cpp \
    changehierarchybutton.cpp \
    cornerwidget.cpp \
    extendedtabbar.cpp \
    playlist.cpp \
    playlistheaderview.cpp \
    playlistitemdelegate.cpp \
    playlistmanager.cpp \
    playlistmodel.cpp \
    tabbar.cpp \
    tabplaylist.cpp \
    stareditor.cpp \
    viewplaylists.cpp

HEADERS += dialogs/closeplaylistpopup.h \
    dialogs/paintablewidget.h \
    dialogs/playlistdialog.h \
    dialogs/searchdialog.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbardirectorylist.h \
    filesystem/addressbarlineedit.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
    mediabuttons/playbackmodebutton.h \
    abstractviewplaylists.h \
    changehierarchybutton.h \
    cornerwidget.h \
    extendedtabbar.h \
    extendedtabwidget.h \
    nofocusitemdelegate.h \
    playlist.h \
    playlistheaderview.h \
    playlistitemdelegate.h \
    playlistmanager.h \
    playlistmodel.h \
    tabbar.h \
    tabplaylist.h \
    stareditor.h \
    miamtabPlaylists_global.hpp \
    viewplaylists.h

FORMS += closeplaylistpopup.ui \
    playlistdialog.ui \
    searchdialog.ui \
    viewplaylists.ui

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../Core/debug/ -lCore
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG += c++11
CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../Core/release/ -lCore -L$$OUT_PWD/../Library/release/ -lLibrary
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
win32 {
    TARGET = TabPlaylists
}
unix {
    LIBS += -L$$OUT_PWD/../Core/ -lmiam-core -L$$OUT_PWD/../Library/ -lmiam-library
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

INCLUDEPATH += $$PWD/../Core/ $$PWD/../Library/
DEPENDPATH += $$PWD/../Core $$PWD/../Library/

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
