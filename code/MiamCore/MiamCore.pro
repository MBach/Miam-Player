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
    LIBS += -L$$OUT_PWD -ltag -lvlc-qt -lvlc-qt-widgets
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    LIBS += -L$$PWD/../../lib/ -ltag -lvlc-qt -lvlc-qt-widgets
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
}

SOURCES += model/librarysqlmodel.cpp \
    model/remoteobject.cpp \
    model/remotetrack.cpp \
    model/selectedtracksmodel.cpp \
    model/sqldatabase.cpp \
    cover.cpp \
    filehelper.cpp \
    flowlayout.cpp \
    mediabutton.cpp \
    mediaplayer.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp \
    settingsprivate.cpp \
    timelabel.cpp \
    model/remoteplaylist.cpp

HEADERS += interfaces/basicplugin.h \
    interfaces/itemviewplugin.h \
    interfaces/mediaplayerplugin.h \
    interfaces/remotemediaplayerplugin.h \
    interfaces/searchmediaplayerplugin.h \
    model/librarysqlmodel.h \
    model/remoteobject.h \
    model/remotetrack.h \
    model/selectedtracksmodel.h \
    model/sqldatabase.h \
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
    timelabel.h \
    model/remoteplaylist.h
