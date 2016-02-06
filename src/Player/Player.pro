QT += widgets multimedia sql

TEMPLATE = app

include(qtsingleapplication/qtsingleapplication.pri)

SOURCES += debug/logbrowser.cpp \
    debug/logbrowserdialog.cpp \
    dialogs/colordialog.cpp \
    dialogs/customizeoptionsdialog.cpp \
    dialogs/customizethemedialog.cpp \
    dialogs/customizethemetaglineedit.cpp \
    dialogs/dragdropdialog.cpp \
    dialogs/equalizerdalog.cpp \
    dialogs/reflector.cpp \
    dialogs/starswidget.cpp \
    styling/miamstyle.cpp \
    views/tageditor/albumcover.cpp \
    views/tageditor/tagconverter.cpp \
    views/tageditor/tageditor.cpp \
    views/tageditor/tageditortablewidget.cpp \
    views/viewloader.cpp \
    columnutils.cpp \
    main.cpp \
    mainwindow.cpp \
    pluginmanager.cpp \
    quickstart.cpp \
    tagbutton.cpp \
    taglineedit.cpp \
    tracksnotfoundmessagebox.cpp

HEADERS += debug/logbrowser.h \
    debug/logbrowserdialog.h \
    dialogs/colordialog.h \
    dialogs/customizeoptionsdialog.h \
    dialogs/customizethemedialog.h \
    dialogs/customizethemetaglineedit.h \
    dialogs/dragdropdialog.h \
    dialogs/equalizerdalog.h \
    dialogs/reflector.h \
    dialogs/starswidget.h \
    styling/miamstyle.h \
    views/tageditor/albumcover.h \
    views/tageditor/tagconverter.h \
    views/tageditor/tageditor.h \
    views/tageditor/tageditortablewidget.h \
    views/viewloader.h \
    columnutils.h \
    mainwindow.h \
    pluginmanager.h \
    quickstart.h \
    tagbutton.h \
    taglineedit.h \
    tracksnotfoundmessagebox.h

FORMS += customizeoptionsdialog.ui \
    customizetheme.ui \
    dragdroppopup.ui \
    equalizerdialog.ui \
    mainwindow.ui \
    quickstart.ui \
    tagconverter.ui \
    tageditor.ui

RESOURCES += player.qrc
CONFIG += c++11
win32 {
    OTHER_FILES += config/mp.rc
    RC_FILE += config/mp.rc
    TARGET = MiamPlayer
}
unix:!macx {
    TARGET = miam-player
}
macx {
    TARGET = MiamPlayer
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

TRANSLATIONS = translations/player_ar.ts \
    translations/player_cs.ts \
    translations/player_de.ts \
    translations/player_en.ts \
    translations/player_es.ts \
    translations/player_fr.ts \
    translations/player_in.ts \
    translations/player_it.ts \
    translations/player_ja.ts \
    translations/player_kr.ts \
    translations/player_pt.ts \
    translations/player_ru.ts \
    translations/player_th.ts \
    translations/player_vn.ts \
    translations/player_zh.ts

CONFIG(debug, debug|release) {
    win32 {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag
	    LIBS += -L$$OUT_PWD/../Core/debug/ -lCore
	    LIBS += -L$$OUT_PWD/../Library/debug/ -lLibrary
	    LIBS += -L$$OUT_PWD/../TabPlaylists/debug/ -lTabPlaylists
	    LIBS += -L$$OUT_PWD/../UniqueLibrary/debug/ -lUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/../Core/mp.ico) $$shell_path($$OUT_PWD/debug/)
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag
	    LIBS += -L$$OUT_PWD/../Core/debug/ -lCore
	    LIBS += -L$$OUT_PWD/../Library/debug/ -lLibrary
	    LIBS += -L$$OUT_PWD/../TabPlaylists/debug/ -lTabPlaylists
	    LIBS += -L$$OUT_PWD/../UniqueLibrary/debug/ -lUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/../Core/mp.ico) $$shell_path($$OUT_PWD/debug/)
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
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag
	    LIBS += -L$$OUT_PWD/../Core/release/ -lCore
	    LIBS += -L$$OUT_PWD/../Library/release/ -lLibrary
	    LIBS += -L$$OUT_PWD/../TabPlaylists/release/ -lTabPlaylists
	    LIBS += -L$$OUT_PWD/../UniqueLibrary/release/ -lUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/../Core/mp.ico) $$shell_path($$OUT_PWD/release/)
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag
	    LIBS += -L$$OUT_PWD/../Core/release/ -lCore
	    LIBS += -L$$OUT_PWD/../Library/release/ -lLibrary
	    LIBS += -L$$OUT_PWD/../TabPlaylists/release/ -lTabPlaylists
	    LIBS += -L$$OUT_PWD/../UniqueLibrary/release/ -lUniqueLibrary
	    QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($$PWD/../Core/mp.ico) $$shell_path($$OUT_PWD/release/)
	}
    }
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = $$PWD
}
unix {
    LIBS += -ltag -L$$OUT_PWD/../Core/ -lmiam-core
    LIBS += -L$$OUT_PWD/../Library/ -lmiam-library
    LIBS += -L$$OUT_PWD/../TabPlaylists/ -lmiam-tabplaylists
    LIBS += -L$$OUT_PWD/../UniqueLibrary/ -lmiam-uniquelibrary
}
unix:!macx {
    target.path = /usr/bin
    desktop.path = /usr/share/applications
    desktop.files = $$PWD/../../debian/usr/share/applications/miam-player.desktop
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    icon64.files = $$PWD/../../debian/usr/share/icons/hicolor/64x64/apps/application-x-miamplayer.png
    appdata.path = /usr/share/appdata
    appdata.files = $$PWD/../../fedora/miam-player.appdata.xml
    INSTALLS += desktop \
	target \
	icon64 \
	appdata
}
macx {
    ICON = $$PWD/../../osx/MiamPlayer.icns
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    #1 create Framework and PlugIns directories
    #2 copy third party library: TagLib, QtAV
    #3 copy own libs
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_MKDIR} $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/PlugIns/) && \
     $${QMAKE_COPY} $$shell_path($$PWD/../../lib/osx/libtag.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../Core/libmiam-core.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../Library/libmiam-library.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../TabPlaylists/libmiam-tabplaylists.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/) && \
     $${QMAKE_COPY} $$shell_path($$OUT_PWD/../UniqueLibrary/libmiam-uniquelibrary.*.dylib) $$shell_path($$OUT_PWD/MiamPlayer.app/Contents/Frameworks/)
}

3rdpartyDir  = $$PWD/../Core/3rdparty
INCLUDEPATH += $$3rdpartyDir
DEPENDPATH += $$3rdpartyDir

INCLUDEPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/playlists $$PWD/views $$PWD/views/tageditor
INCLUDEPATH += $$PWD/../Core
INCLUDEPATH += $$PWD/../Library
INCLUDEPATH += $$PWD/../TabPlaylists
INCLUDEPATH += $$PWD/../UniqueLibrary

DEPENDPATH += $$PWD/dialogs $$PWD/filesystem $$PWD/playlists $$PWD/views $$PWD/views/tageditor
DEPENDPATH += $$PWD/../Core
DEPENDPATH += $$PWD/../Library
DEPENDPATH += $$PWD/../TabPlaylists
DEPENDPATH += $$PWD/../UniqueLibrary
