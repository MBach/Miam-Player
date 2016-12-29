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
    viewplaylists.cpp \
    viewplaylistsmediaplayercontrol.cpp

HEADERS += dialogs/closeplaylistpopup.h \
    dialogs/playlistdialog.h \
    dialogs/searchdialog.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbardirectorylist.h \
    filesystem/addressbarlineedit.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
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
    viewplaylists.h \
    viewplaylistsmediaplayercontrol.h \
    miamtabplaylists_global.hpp

FORMS += closeplaylistpopup.ui \
    customtags.ui \
    playlistdialog.ui \
    searchdialog.ui \
    viewplaylists.ui
CONFIG += c++11
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

TARGET = miam-tabplaylists

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

RESOURCES += tabPlaylists.qrc

TRANSLATIONS = translations/tabPlaylists_ar.ts \
    translations/tabPlaylists_cs.ts \
    translations/tabPlaylists_de.ts \
    translations/tabPlaylists_el.ts \
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
