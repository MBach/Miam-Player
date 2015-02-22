QT       += widgets multimedia sql

3rdpartyDir  = $$PWD/3rdparty

INCLUDEPATH += $$PWD
INCLUDEPATH += $$3rdpartyDir
DEPENDPATH += $$3rdpartyDir

DEFINES += MIAM_PLUGIN

TEMPLATE = lib

win32 {
    TARGET = MiamCore
    CONFIG += dll
    CONFIG(debug, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag -L$$PWD/../../lib/debug/win-x86/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag -L$$PWD/../../lib/debug/win-x64/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	}
    }
    CONFIG(release, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag -L$$PWD/../../lib/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	    LIBS += -L$$PWD/../../lib/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag -L$$PWD/../../lib/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	    LIBS += -L$$PWD/../../lib/vlc-qt/ -lvlc-qt -lvlc-qt-widgets
	}
    }
}
unix {
    TARGET = miam-core
    CONFIG += c++11
    QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    LIBS += -L$$OUT_PWD -L/usr/local/ -ltag -lvlc-qt -lvlc-qt-widgets
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    LIBS += -L$$PWD/../../lib/ -ltag -lvlc-qt -lvlc-qt-widgets
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
}

SOURCES +=     library/jumptowidget.cpp \
    model/albumdao.cpp \
    model/artistdao.cpp \
    model/genericdao.cpp \
    model/playlistdao.cpp \
    model/selectedtracksmodel.cpp \
    model/sqldatabase.cpp \
    model/trackdao.cpp \
    model/yeardao.cpp \
    cover.cpp \
    filehelper.cpp \
    flowlayout.cpp \
    mediabutton.cpp \
    mediaplayer.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp \
    settingsprivate.cpp \
    timelabel.cpp

HEADERS += interfaces/basicplugin.h \
    interfaces/itemviewplugin.h \
    interfaces/mediaplayerplugin.h \
    interfaces/remotemediaplayerplugin.h \
    interfaces/searchmediaplayerplugin.h \
    library/jumptowidget.h \
    model/albumdao.h \
    model/artistdao.h \
    model/genericdao.h \
    model/playlistdao.h \
    model/selectedtracksmodel.h \
    model/sqldatabase.h \
    model/trackdao.h \
    model/yeardao.h \
    abstractsearchdialog.h \
    cover.h \
    filehelper.h \
    flowlayout.h \
    mediabutton.h \
    mediaplayer.h \
    miamcore_global.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    remotemediaplayer.h \
    settings.h \
    settingsprivate.h \
    timelabel.h
