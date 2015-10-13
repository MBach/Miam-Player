QT       += widgets multimedia sql

3rdpartyDir  = $$PWD/3rdparty

INCLUDEPATH += $$PWD
INCLUDEPATH += $$3rdpartyDir $$3rdpartyDir/QtAV
DEPENDPATH += $$3rdpartyDir $$3rdpartyDir/QtAV

DEFINES += MIAM_PLUGIN

TEMPLATE = lib

win32 {
    TARGET = MiamCore
    CONFIG += dll
    CONFIG(debug, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag -lQtAV1
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag -lQtAV1
	}
    }
    CONFIG(release, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag -lQtAV1
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag -lQtAV1
	}
    }
}

# intermediate objects are put in subdirs
CONFIG(debug, debug|release) {
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
}
CONFIG(release, debug|release) {
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
}
CONFIG += c++11
unix {
    TARGET = miam-core
    QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    LIBS += -L$$OUT_PWD -L/usr/lib/x86_64-linux-gnu/ -ltag
    # XXX
    isEqual(QT_MAJOR_VERSION, 5):isEqual(QT_MINOR_VERSION, 4):lessThan(QT_PATCH_VERSION, 2){
	LIBS += -lQt5AV
    } else {
	LIBS += -lQtAV
    }
    target.path = /usr/lib/
    INSTALLS += target
}
macx {
    LIBS += -L$$PWD/../../lib/osx/ -ltag -lQtAV1
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_PRE_LINK = rm -f $$OUT_PWD/../MiamPlayer/MiamPlayer.app/Contents/MacOS/MiamPlayer
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
}

SOURCES += library/jumptowidget.cpp \
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
    mediaplaylist.cpp \
    musicsearchengine.cpp \
    quickstartsearchengine.cpp \
    settings.cpp \
    settingsprivate.cpp \
    stopbutton.cpp \
    timelabel.cpp \
    treeview.cpp \
    styling/imageutils.cpp \
    styling/lineedit.cpp \
    starrating.cpp \
    miamsortfilterproxymodel.cpp \
    plugininfo.cpp

HEADERS += interfaces/basicplugin.h \
    interfaces/itemviewplugin.h \
    interfaces/mediaplayerplugin.h \
    interfaces/remotemediaplayerplugin.h \
    interfaces/searchmediaplayerplugin.h \
    interfaces/tageditorplugin.h \
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
    imediaplayer.h \
    mediabutton.h \
    mediaplayer.h \
    mediaplaylist.h \
    miamcore_global.h \
    musicsearchengine.h \
    quickstartsearchengine.h \
    settings.h \
    settingsprivate.h \
    stopbutton.h \
    timelabel.h \
    treeview.h \
    styling/imageutils.h \
    styling/lineedit.h \
    starrating.h \
    searchbar.h \
    miamsortfilterproxymodel.h \
    plugininfo.h
