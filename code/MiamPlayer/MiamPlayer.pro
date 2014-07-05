QT += widgets multimedia sql

TEMPLATE = app

SOURCES += \
    dialogs/closeplaylistpopup.cpp \
    dialogs/colordialog.cpp \
    dialogs/customizeoptionsdialog.cpp \
    dialogs/customizethemedialog.cpp \
    dialogs/dragdropdialog.cpp \
    dialogs/playlistmanager.cpp \
    dialogs/reflector.cpp \
    filesystem/addressbar.cpp \
    filesystem/addressbarbutton.cpp \
    filesystem/addressbarmenu.cpp \
    filesystem/filesystemtreeview.cpp \
    library/extendedtabbar.cpp \
    library/jumptowidget.cpp \
    library/libraryfilterproxymodel.cpp \
    library/libraryheader.cpp \
    library/libraryitemdelegate.cpp \
    library/libraryorderdialog.cpp \
    library/libraryscrollbar.cpp \
    library/librarytreeview.cpp \
    playlists/playlist.cpp \
    playlists/playlistheaderview.cpp \
    playlists/playlistmodel.cpp \
    playlists/tabbar.cpp \
    playlists/tabplaylist.cpp \
    playlists/playlistitemdelegate.cpp \
    playlists/starrating.cpp \
    playlists/stareditor.cpp \
    styling/miamstyleditemdelegate.cpp \
    tageditor/albumcover.cpp \
    tageditor/tagconverter.cpp \
    tageditor/tageditor.cpp \
    tageditor/tageditortablewidget.cpp \
    circleprogressbar.cpp \
    columnutils.cpp \
    main.cpp \
    mainwindow.cpp \
    miamstyle.cpp \
    playbackmodewidget.cpp \
    playbackmodewidgetfactory.cpp \
    plugininfo.cpp \
    pluginmanager.cpp \
    quickstart.cpp \
    scrollbar.cpp \
    seekbar.cpp \
    tracksnotfoundmessagebox.cpp \
    treeview.cpp \
    volumeslider.cpp \
    taglineedit.cpp \
    tagbutton.cpp \
    styling/lineedit.cpp \
    libraryfilterlineedit.cpp \
    localserver.cpp \
    singleapplication.cpp

HEADERS += \
    dialogs/closeplaylistpopup.h \
    dialogs/colordialog.h \
    dialogs/customizeoptionsdialog.h \
    dialogs/customizethemedialog.h \
    dialogs/dragdropdialog.h \
    dialogs/playlistmanager.h \
    dialogs/reflector.h \
    dialogs/paintablewidget.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
    library/extendedtabbar.h \
    library/extendedtabwidget.h \
    library/libraryfilterproxymodel.h \
    library/libraryitemdelegate.h \
    library/libraryorderdialog.h \
    library/libraryscrollbar.h \
    library/librarytreeview.h \
    library/libraryheader.h \
    library/jumptowidget.h \
    playlists/playlist.h \
    playlists/playlistheaderview.h \
    playlists/playlistmodel.h \
    playlists/tabbar.h \
    playlists/tabplaylist.h \
    playlists/playlistitemdelegate.h \
    playlists/starrating.h \
    playlists/stareditor.h \
    playlists/playlistframe.h \
    styling/miamstyleditemdelegate.h \
    tageditor/albumcover.h \
    tageditor/tagconverter.h \
    tageditor/tageditor.h \
    tageditor/tageditortablewidget.h \
    circleprogressbar.h \
    columnutils.h \
    mainwindow.h \
    miamstyle.h \
    nofocusitemdelegate.h \
    playbackmodewidget.h \
    playbackmodewidgetfactory.h \
    pluginmanager.h \
    plugininfo.h \
    quickstart.h \
    scrollbar.h \
    seekbar.h \
    tracksnotfoundmessagebox.h \
    treeview.h \
    volumeslider.h \
    taglineedit.h \
    tagbutton.h \
    styling/lineedit.h \
    libraryfilterlineedit.h \
    localserver.h \
    singleapplication.h

FORMS += \
    mainwindow.ui \
    customizeoptionsdialog.ui \
    customizetheme.ui \
    dragdroppopup.ui \
    libraryorderdialog.ui \
    playlistmanager.ui \
    quickstart.ui \
    tagconverter.ui \
    tageditor.ui \
    closeplaylistpopup.ui

RESOURCES += \
    mp.qrc

win32 {
    RC_FILE += config/mp.rc
    OTHER_FILES +=
}
unix {
    CONFIG += c++11
    QMAKE_CXXFLAGS += -std=c++11
}
macx {
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
}

TRANSLATIONS = translations/m4p_ar.ts \
    translations/m4p_cs.ts \
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
    win32 {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/debug/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/debug/)
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/debug/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/debug/)
	}
    }
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = $$PWD
}

CONFIG(release, debug|release) {
    win32 {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/release/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/release/)
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/release/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/release/)
	}
    }
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
unix:!macx {
    LIBS += -ltag -L$$OUT_PWD/../MiamCore/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/ -lMiamUniqueLibrary
}
macx {
    LIBS += -L$$PWD/../../lib/ -ltag -L$$OUT_PWD/../MiamCore/ -lMiamCore -L$$OUT_PWD/../MiamUniqueLibrary/ -lMiamUniqueLibrary
    ICON = $$PWD/mp.icns
    QMAKE_INFO_PLIST = $$PWD/../../packaging/osx/Info.plist
    #1 create Framework directory
    #2 copy third party library: TagLib
    #3 copy own libs
    #4 execute macdeploy to create a nice bundle
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
      $${QMAKE_COPY} $$shell_path($$PWD/../../lib/libtag.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
      $${QMAKE_COPY} $$shell_path($$OUT_PWD/../MiamCore/libMiamCore.1.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
      $${QMAKE_COPY} $$shell_path($$OUT_PWD/../MiamUniqueLibrary/libMiamUniqueLibrary.1.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
      $${QMAKESPEC}/../../bin/macdeployqt $$OUT_PWD/MiamPlayer.app
}

INCLUDEPATH += $$PWD/../MiamCore
INCLUDEPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/library $$PWD/playlists $$PWD/tageditor
INCLUDEPATH += $$PWD/../MiamUniqueLibrary

DEPENDPATH += $$PWD/../MiamCore
DEPENDPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/library $$PWD/playlists $$PWD/tageditor
DEPENDPATH += $$PWD/../MiamUniqueLibrary

OTHER_FILES += \
    config/mp.rc
