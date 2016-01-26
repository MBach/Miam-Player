QT       += widgets multimedia sql

3rdpartyDir  = $$PWD/3rdparty

INCLUDEPATH += $$PWD
INCLUDEPATH += $$3rdpartyDir $$3rdpartyDir/QtAV
DEPENDPATH += $$3rdpartyDir $$3rdpartyDir/QtAV

include(qxt/qxt.pri)

DEFINES += MIAM_PLUGIN

TEMPLATE = lib

win32 {
    TARGET = Core
    CONFIG += dll
    CONFIG(debug, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/debug/win-x86/ -ltag -lQtAV1 -lUser32
	} else {
	    LIBS += -L$$PWD/../../lib/debug/win-x64/ -ltag -lQtAV1 -lUser32
	}
    }
    CONFIG(release, debug|release) {
	!contains(QMAKE_TARGET.arch, x86_64) {
	    LIBS += -L$$PWD/../../lib/release/win-x86/ -ltag -lQtAV1 -lUser32
	} else {
	    LIBS += -L$$PWD/../../lib/release/win-x64/ -ltag -lQtAV1 -lUser32
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
    #QMAKE_CXXFLAGS += -std=c++11
}
unix:!macx {
    QT += x11extras
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
    #auto clean
    QMAKE_PRE_LINK = rm -f $$OUT_PWD/../Player/MiamPlayer.app
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    QMAKE_LFLAGS += -F$$PWD/../../lib/osx/QtAV.framework -F/System/Library/Frameworks/Carbon.framework/
    LIBS += -L$$PWD/../../lib/osx/ -ltag -framework QtAV -framework Carbon
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

SOURCES += library/jumptowidget.cpp \
    mediabuttons/mediabutton.cpp \
    mediabuttons/stopbutton.cpp \
    model/albumdao.cpp \
    model/artistdao.cpp \
    model/genericdao.cpp \
    model/playlistdao.cpp \
    model/selectedtracksmodel.cpp \
    model/sqldatabase.cpp \
    model/trackdao.cpp \
    model/yeardao.cpp \
    styling/imageutils.cpp \
    styling/lineedit.cpp \
    styling/miamslider.cpp \
    styling/miamstyleditemdelegate.cpp \
    widgets/seekbar.cpp \
    widgets/timelabel.cpp \
    widgets/volumeslider.cpp \
    cover.cpp \
    filehelper.cpp \
    flowlayout.cpp \
    mediaplayer.cpp \
    mediaplaylist.cpp \
    miamsortfilterproxymodel.cpp \
    musicsearchengine.cpp \
    plugininfo.cpp \
    quickstartsearchengine.cpp \
    scrollbar.cpp \
    settings.cpp \
    settingsprivate.cpp \
    starrating.cpp \
    treeview.cpp

HEADERS += interfaces/basicplugin.h \
    interfaces/itemviewplugin.h \
    interfaces/mediaplayerplugin.h \
    interfaces/remotemediaplayerplugin.h \
    interfaces/tageditorplugin.h \
    library/jumptowidget.h \
    mediabuttons/mediabutton.h \
    mediabuttons/stopbutton.h \
    model/albumdao.h \
    model/artistdao.h \
    model/genericdao.h \
    model/playlistdao.h \
    model/selectedtracksmodel.h \
    model/sqldatabase.h \
    model/trackdao.h \
    model/yeardao.h \
    styling/imageutils.h \
    styling/lineedit.h \
    styling/miamslider.h \
    styling/miamstyleditemdelegate.h \
    widgets/seekbar.h \
    widgets/timelabel.h \
    widgets/volumeslider.h \
    abstractsearchdialog.h \
    abstractview.h \
    cover.h \
    filehelper.h \
    flowlayout.h \
    imediaplayer.h \
    mediaplayer.h \
    mediaplaylist.h \
    miamcore_global.h \
    miamsortfilterproxymodel.h \
    musicsearchengine.h \
    plugininfo.h \
    quickstartsearchengine.h \
    scrollbar.h \
    searchbar.h \
    settings.h \
    settingsprivate.h \
    starrating.h \
    treeview.h

RESOURCES += core.qrc
