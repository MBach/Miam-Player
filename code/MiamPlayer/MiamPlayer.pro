QT += widgets multimedia sql

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    filesystem/addressbar.cpp \
    filesystem/addressbarbutton.cpp \
    filesystem/addressbarmenu.cpp \
    filesystem/filesystemtreeview.cpp \
    treeview.cpp \
    library/libraryfilterproxymodel.cpp \
    library/libraryitemdelegate.cpp \
    library/libraryorderdialog.cpp \
    library/librarytreeview.cpp \
    circleprogressbar.cpp \
    mediabutton.cpp \
    tracksnotfoundmessagebox.cpp \
    quickstart.cpp \
    timelabel.cpp \
    columnutils.cpp \
    playbackmodewidget.cpp \
    playbackmodewidgetfactory.cpp \
    playlists/playlist.cpp \
    playlists/playlistheaderview.cpp \
    playlists/playlistmodel.cpp \
    playlists/stardelegate.cpp \
    playlists/stareditor.cpp \
    playlists/starrating.cpp \
    playlists/tabbar.cpp \
    playlists/tabplaylist.cpp \
    dialogs/colordialog.cpp \
    dialogs/customizeoptionsdialog.cpp \
    dialogs/customizethemedialog.cpp \
    dialogs/dragdropdialog.cpp \
    dialogs/playlistmanager.cpp \
    dialogs/reflector.cpp \
    dialogs/shortcutlineedit.cpp \
    dialogs/shortcutwidget.cpp \
    dialogs/stylesheetupdater.cpp \
    tageditor/albumcover.cpp \
    tageditor/tagconverter.cpp \
    tageditor/tageditor.cpp \
    tageditor/tageditortablewidget.cpp

HEADERS += \
    mainwindow.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
    treeview.h \
    library/extendedtabbar.h \
    library/extendedtabwidget.h \
    library/libraryfilterproxymodel.h \
    library/libraryitemdelegate.h \
    library/libraryorderdialog.h \
    library/librarytreeview.h \
    circleprogressbar.h \
    mediabutton.h \
    tracksnotfoundmessagebox.h \
    quickstart.h \
    timelabel.h \
    columnutils.h \
    nofocusitemdelegate.h \
    playbackmodewidget.h \
    playbackmodewidgetfactory.h \
    playlists/playlist.h \
    playlists/playlistheaderview.h \
    playlists/playlistmodel.h \
    playlists/stardelegate.h \
    playlists/stareditor.h \
    playlists/starrating.h \
    playlists/tabbar.h \
    playlists/tabplaylist.h \
    dialogs/colordialog.h \
    dialogs/customizeoptionsdialog.h \
    dialogs/customizethemedialog.h \
    dialogs/dragdropdialog.h \
    dialogs/playlistmanager.h \
    dialogs/reflector.h \
    dialogs/shortcutlineedit.h \
    dialogs/shortcutwidget.h \
    dialogs/stylesheetupdater.h \
    tageditor/albumcover.h \
    tageditor/tagconverter.h \
    tageditor/tageditor.h \
    tageditor/tageditortablewidget.h

FORMS += \
    mainwindow.ui \
    customizeoptionsdialog.ui \
    customizetheme.ui \
    dragdroppopup.ui \
    libraryorderdialog.ui \
    playlistmanager.ui \
    quickstart.ui \
    tagconverter.ui \
    tageditor.ui

OTHER_FILES += \
    stylesheets/qscrollbar.qss \
    stylesheets/librarytreeview.qss \
    stylesheets/qslider.qss \
    stylesheets/tageditor.qss \
    stylesheets/playlist.qss

RESOURCES += \
    mmmmp.qrc

win32 {
    RC_FILE += config/mmmmp.rc
    OTHER_FILES += config/mmmmp.rc
}

TRANSLATIONS = translations/m4p_ar.ts \
    translations/m4p_de.ts \
    translations/m4p_en.ts \
    translations/m4p_es.ts \
    translations/m4p_fr.ts \
    translations/m4p_it.ts \
    translations/m4p_ja.ts \
    translations/m4p_kr.ts \
    translations/m4p_pt.ts \
    translations/m4p_ru.ts \
    translations/m4p_th.ts \
    translations/m4p_vn.ts \
    translations/m4p_zh.ts

CONFIG(debug, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/debug/ -L$$OUT_PWD/../MiamUniqueLibrary/debug/ -llibtag -lMiamCore -lMiamUniqueLibrary
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}

CONFIG(release, debug|release) {
    win32: LIBS += -L$$OUT_PWD/../MiamCore/release/ -L$$OUT_PWD/../MiamUniqueLibrary/release/ -llibtag -lMiamCore -lMiamUniqueLibrary
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}
unix: LIBS += -L$$OUT_PWD/../MiamCore/ -L$$OUT_PWD/../MiamUniqueLibrary/ -ltag -lMiamCore -lMiamUniqueLibrary

INCLUDEPATH += $$PWD/../MiamCore
INCLUDEPATH += $$PWD/../MiamUniqueLibrary

DEPENDPATH += $$PWD/../MiamCore
DEPENDPATH += $$PWD/../MiamUniqueLibrary
