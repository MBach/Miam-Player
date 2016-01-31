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
    stareditor.cpp \
    tabbar.cpp \
    tabplaylist.cpp \
    viewplaylists.cpp

HEADERS += dialogs/closeplaylistpopup.h \
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
    miamtabPlaylists_global.hpp \
    nofocusitemdelegate.h \
    playlist.h \
    playlistheaderview.h \
    playlistitemdelegate.h \
    playlistmanager.h \
    playlistmodel.h \
    stareditor.h \
    tabbar.h \
    tabplaylist.h \
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

RESOURCES += tabPlaylists.qrc

TRANSLATIONS = translations/tabPlaylists_ar.ts \
    translations/tabPlaylists_cs.ts \
    translations/tabPlaylists_de.ts \
    translations/tabPlaylists_en.ts \
    translations/tabPlaylists_es.ts \
    translations/tabPlaylists_fr.ts \
    translations/tabPlaylists_in.ts \
    translations/tabPlaylists_it.ts \
    translations/tabPlaylists_ja.ts \
    translations/tabPlaylists_kr.ts \
    translations/tabPlaylists_pt.ts \
    translations/tabPlaylists_ru.ts \
    translations/tabPlaylists_th.ts \
    translations/tabPlaylists_vn.ts \
    translations/tabPlaylists_zh.ts
