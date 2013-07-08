QT += widgets multimedia

TagLibDirectory = ./3rdparty/taglib

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

FORMS += \
    customizeoptionsdialog.ui \
    customizetheme.ui \
    mainwindow.ui \
    playlistmanager.ui \
    tagconverter.ui \
    tageditor.ui \
    dragdroppopup.ui \
    quickstart.ui

SOURCES += \
    dialogs/colordialog.cpp \
    dialogs/customizethemedialog.cpp \
    dialogs/customizeoptionsdialog.cpp \
    dialogs/dragdropdialog.cpp \
    dialogs/playlistmanager.cpp \
    dialogs/reflector.cpp \
    dialogs/shortcutlineedit.cpp \
    dialogs/shortcutwidget.cpp \
    dialogs/stylesheetupdater.cpp \
    filesystem/addressbar.cpp \
    filesystem/addressbarbutton.cpp \
    filesystem/addressbarmenu.cpp \
    filesystem/filesystemtreeview.cpp \
    library/libraryfilterlineedit.cpp \
    library/libraryfilterproxymodel.cpp \
    library/libraryitem.cpp \
    library/libraryitemdelegate.cpp \
    library/librarymodel.cpp \
    library/librarytreeview.cpp \
    library/musicsearchengine.cpp \
    playlists/playlist.cpp \
    playlists/playlistmodel.cpp \
    playlists/stardelegate.cpp \
    playlists/stareditor.cpp \
    playlists/starrating.cpp \
    playlists/tabbar.cpp \
    playlists/tabplaylist.cpp \
    tageditor/albumcover.cpp \
    tageditor/cover.cpp \
    tageditor/tagconverter.cpp \
    tageditor/tageditor.cpp \
    tageditor/tageditortablewidget.cpp \
    circleprogressbar.cpp \
    columnutils.cpp \
    filehelper.cpp \
    main.cpp \
    mainwindow.cpp \
    mediabutton.cpp \
    playbackmodewidget.cpp \
    playbackmodewidgetfactory.cpp \
    quickstart.cpp \
    settings.cpp \
    timelabel.cpp \
    tracksnotfoundmessagebox.cpp \
    treeview.cpp

HEADERS += \
    dialogs/colordialog.h \
    dialogs/customizethemedialog.h \
    dialogs/customizeoptionsdialog.h \
    dialogs/dragdropdialog.h \
    dialogs/playlistmanager.h \
    dialogs/reflector.h \
    dialogs/shortcutlineedit.h \
    dialogs/shortcutwidget.h \
    dialogs/stylesheetupdater.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
    library/extendedtabbar.h \
    library/extendedtabwidget.h \
    library/libraryfilterlineedit.h \
    library/libraryfilterproxymodel.h \
    library/libraryitem.h \
    library/libraryitemdelegate.h \
    library/librarymodel.h \
    library/librarytreeview.h \
    library/musicsearchengine.h \
    playlists/playlist.h \
    playlists/playlistmodel.h \
    playlists/stardelegate.h \
    playlists/stareditor.h \
    playlists/starrating.h \
    playlists/tabbar.h \
    playlists/tabplaylist.h \
    tageditor/albumcover.h \
    tageditor/cover.h \
    tageditor/tagconverter.h \
    tageditor/tageditor.h \
    tageditor/tageditortablewidget.h \
    circleprogressbar.h \
    columnutils.h \
    filehelper.h \
    mainwindow.h \
    mediabutton.h \
    nofocusitemdelegate.h \
    playbackmodewidget.h \
    playbackmodewidgetfactory.h \
    quickstart.h \
    settings.h \
    timelabel.h \
    tracksnotfoundmessagebox.h \
    treeview.h

OTHER_FILES += \
    stylesheets/qscrollbar.qss \
    stylesheets/librarytreeview.qss \
    stylesheets/qslider.qss \
    stylesheets/tageditor.qss \
    stylesheets/playlist.qss


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

RESOURCES += \
    mmmmp.qrc

win32 {
    RC_FILE += config/mmmmp.rc
    OTHER_FILES += config/mmmmp.rc
}

CONFIG(debug, debug|release) {
    win32: LIBS += -Ldebug -llibtag
    unix: LIBS += -Ldebug -ltag
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}

CONFIG(release, debug|release) {
    win32: LIBS += -Lrelease -llibtag
    unix: LIBS += -Lrelease -ltag
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}

INCLUDEPATH += $$TagLibDirectory \
    $$TagLibDirectory/include
