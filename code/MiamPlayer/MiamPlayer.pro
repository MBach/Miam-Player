QT += widgets multimedia sql

TEMPLATE = app

include(qtsingleapplication/qtsingleapplication.pri)

SOURCES += \
    debug/logbrowser.cpp \
    debug/logbrowserdialog.cpp \
    dialogs/closeplaylistpopup.cpp \
    dialogs/colordialog.cpp \
    dialogs/customizeoptionsdialog.cpp \
    dialogs/customizethemedialog.cpp \
    dialogs/customizethemetaglineedit.cpp \
    dialogs/dragdropdialog.cpp \
    dialogs/equalizerdalog.cpp \
    dialogs/playlistdialog.cpp \
    dialogs/reflector.cpp \
    dialogs/searchdialog.cpp \
    dialogs/starswidget.cpp \
    filesystem/addressbar.cpp \
    filesystem/addressbarbutton.cpp \
    filesystem/addressbarmenu.cpp \
    filesystem/filesystemtreeview.cpp \
    playlists/cornerwidget.cpp \
    playlists/playlist.cpp \
    playlists/playlistheaderview.cpp \
    playlists/playlistmanager.cpp \
    playlists/playlistmodel.cpp \
    playlists/tabbar.cpp \
    playlists/tabplaylist.cpp \
    playlists/playlistitemdelegate.cpp \
    playlists/stareditor.cpp \
    styling/miamslider.cpp \
    styling/miamstyleditemdelegate.cpp \
    tageditor/albumcover.cpp \
    tageditor/tagconverter.cpp \
    tageditor/tageditor.cpp \
    tageditor/tageditortablewidget.cpp \
    columnutils.cpp \
    main.cpp \
    mainwindow.cpp \
    miamstyle.cpp \
    playbackmodewidget.cpp \
    playbackmodewidgetfactory.cpp \
    plugininfo.cpp \
    pluginmanager.cpp \
    quickstart.cpp \
    seekbar.cpp \
    tagbutton.cpp \
    taglineedit.cpp \
    tracksnotfoundmessagebox.cpp \
    volumeslider.cpp \
    libraryfilterlineedit.cpp

HEADERS += \
    debug/logbrowser.h \
    debug/logbrowserdialog.h \
    dialogs/closeplaylistpopup.h \
    dialogs/colordialog.h \
    dialogs/customizeoptionsdialog.h \
    dialogs/customizethemedialog.h \
    dialogs/customizethemetaglineedit.h \
    dialogs/dragdropdialog.h \
    dialogs/equalizerdalog.h \
    dialogs/paintablewidget.h \
    dialogs/playlistdialog.h \
    dialogs/reflector.h \
    dialogs/searchdialog.h \
    dialogs/starswidget.h \
    filesystem/addressbar.h \
    filesystem/addressbarbutton.h \
    filesystem/addressbarmenu.h \
    filesystem/filesystemtreeview.h \
    playlists/cornerwidget.h \
    playlists/playlist.h \
    playlists/playlistheaderview.h \
    playlists/playlistmanager.h \
    playlists/playlistmodel.h \
    playlists/tabbar.h \
    playlists/tabplaylist.h \
    playlists/playlistitemdelegate.h \
    playlists/stareditor.h \
    playlists/playlistframe.h \
    styling/miamslider.h \
    styling/miamstyleditemdelegate.h \
    tageditor/albumcover.h \
    tageditor/tagconverter.h \
    tageditor/tageditor.h \
    tageditor/tageditortablewidget.h \
    columnutils.h \
    mainwindow.h \
    miamstyle.h \
    nofocusitemdelegate.h \
    playbackmodewidget.h \
    playbackmodewidgetfactory.h \
    pluginmanager.h \
    plugininfo.h \
    quickstart.h \
    seekbar.h \
    tagbutton.h \
    taglineedit.h \
    tracksnotfoundmessagebox.h \
    volumeslider.h \
    libraryfilterlineedit.h

FORMS += closeplaylistpopup.ui \
    customizeoptionsdialog.ui \
    customizetheme.ui \
    dragdroppopup.ui \
    equalizerdialog.ui \
    mainwindow.ui \
    playlistdialog.ui \
    quickstart.ui \
    searchdialog.ui \
    tagconverter.ui \
    tageditor.ui

RESOURCES += mp.qrc

win32 {
    RC_FILE += config/mp.rc
    TARGET = MiamPlayer
}
unix {
    CONFIG += c++11
    QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    TARGET = miam-player
}
macx {
    TARGET = MiamPlayer
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
}

TRANSLATIONS = translations/m4p_ar.ts \
    translations/m4p_cs.ts \
    translations/m4p_de.ts \
    translations/m4p_en.ts \
    translations/m4p_es.ts \
    translations/m4p_fr.ts \
    translations/m4p_in.ts \
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
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/debug/ -lMiamLibrary -L$$OUT_PWD/../MiamUniqueLibrary/debug/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/debug/)
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag -L$$OUT_PWD/../MiamCore/debug/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/debug/ -lMiamLibrary -L$$OUT_PWD/../MiamUniqueLibrary/debug/ -lMiamUniqueLibrary
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
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/release/ -lMiamLibrary -L$$OUT_PWD/../MiamUniqueLibrary/release/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/release/)
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag -L$$OUT_PWD/../MiamCore/release/ -lMiamCore -L$$OUT_PWD/../MiamLibrary/release/ -lMiamLibrary -L$$OUT_PWD/../MiamUniqueLibrary/release/ -lMiamUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/mp.ico) $$shell_path($$OUT_PWD/release/)
	}
    }
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
unix:!macx {
    LIBS += -ltag -L$$OUT_PWD/../MiamCore/ -lmiam-core -L$$OUT_PWD/../MiamLibrary/ -lmiam-library -L$$OUT_PWD/../MiamUniqueLibrary/ -lmiam-uniquelibrary
    target.path = /usr/bin/
    INSTALLS += target
}
macx {
    LIBS += -L$$PWD/../../lib/osx/ -ltag -L$$OUT_PWD/../MiamCore/ -lmiam-core -L$$OUT_PWD/../MiamLibrary/ -lmiam-library -L$$OUT_PWD/../MiamUniqueLibrary/ -lmiam-uniquelibrary
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    #1 create Framework directory
    #2 copy third party library: TagLib, VLC-Qt
    #3 copy own libs
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$PWD/../../lib/osx/libtag.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../MiamCore/libmiam-core.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../MiamUniqueLibrary/libmiam-uniquelibrary.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/)
}

3rdpartyDir  = $$PWD/../MiamCore/3rdparty
INCLUDEPATH += $$3rdpartyDir
DEPENDPATH += $$3rdpartyDir

INCLUDEPATH += $$PWD/../MiamCore
INCLUDEPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/playlists $$PWD/tageditor
INCLUDEPATH += $$PWD/../MiamLibrary $$PWD/../MiamUniqueLibrary

DEPENDPATH += $$PWD/../MiamCore
DEPENDPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/playlists $$PWD/tageditor
DEPENDPATH += $$PWD/../MiamLibrary $$PWD/../MiamUniqueLibrary

OTHER_FILES += config/mp.rc \
    qtsingleapplication/qtsingleapplication.pri
