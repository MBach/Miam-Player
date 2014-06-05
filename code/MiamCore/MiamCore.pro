QT       += widgets multimedia sql

TagLibDirectory = $$PWD/3rdparty/taglib
INCLUDEPATH += $$PWD
INCLUDEPATH += $$TagLibDirectory
DEPENDPATH += $$TagLibDirectory

DEFINES += MIAM_PLUGIN

TARGET = MiamCore
TEMPLATE = lib

win32 {
    CONFIG += dll
    CONFIG(debug, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag
	}
    }
    CONFIG(release, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag
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
    model/selectedtracksmodel.cpp \
    cover.cpp \
    filehelper.cpp \
    libraryfilterlineedit.cpp \
    mediabutton.cpp \
    mediaplayer.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp \
    sqldatabase.cpp \
    timelabel.cpp

HEADERS += interfaces/basicplugininterface.h \
    interfaces/itemviewplugininterface.h \
    interfaces/mediaplayerplugininterface.h \
    model/librarysqlmodel.h \
    model/selectedtracksmodel.h \
    cover.h \
    filehelper.h \
    libraryfilterlineedit.h \
    mediabutton.h \
    mediaplayer.h \
    miamcore_global.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    settings.h \
    sqldatabase.h \
    timelabel.h
