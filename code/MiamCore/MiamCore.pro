QT       += widgets multimedia sql

3rdpartyDir  = $$PWD/3rdparty

INCLUDEPATH += $$PWD
INCLUDEPATH += $$3rdpartyDir
DEPENDPATH += $$3rdpartyDir

DEFINES += MIAM_PLUGIN

TARGET = MiamCore
TEMPLATE = lib

win32 {
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
    CONFIG += c++11
    QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    LIBS += -L$$OUT_PWD -ltag
}
macx {
    LIBS += -L$$PWD/../../lib/ -ltag
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
}

SOURCES += \
    model/librarysqlmodel.cpp \
    cover.cpp \
    filehelper.cpp \
    mediabutton.cpp \
    mediaplayer.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp \
    sqldatabase.cpp \
    timelabel.cpp \
    model/selectedtracksmodel.cpp \
    flowlayout.cpp \
    model/remotetrack.cpp \
    remotemediaplayer.cpp

HEADERS += \
    model/librarysqlmodel.h \
    model/selectedtracksmodel.h \
    cover.h \
    filehelper.h \
    mediabutton.h \
    mediaplayer.h \
    miamcore_global.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    settings.h \
    sqldatabase.h \
    timelabel.h \
    flowlayout.h \
    interfaces/searchmediaplayerplugin.h \
    interfaces/basicplugin.h \
    interfaces/itemviewplugin.h \
    interfaces/mediaplayerplugin.h \
    abstractsearchdialog.h \
    model/remotetrack.h \
    remotemediaplayer.h \
    interfaces/remotemediaplayerplugin.h
