QT += phonon

TagLibDirectory = C:/dev/lib/taglib-1.7

TEMPLATE = app

FORMS += \
    mainwindow.ui \
    customizetheme.ui \
    customizeoptionsdialog.ui

SOURCES += \
    main.cpp \
    circleprogressbar.cpp \
    libraryfilterlineedit.cpp \
    libraryfilterproxymodel.cpp \
    librarymodel.cpp \
    librarytreeview.cpp \
    mainwindow.cpp \
    mediabutton.cpp \
    musicsearchengine.cpp \
    playlist.cpp \
    settings.cpp \
    tabplaylist.cpp \
    customizethemedialog.cpp \
    customizeoptionsdialog.cpp \
    libraryitemdelegate.cpp \
    starrating.cpp \
    stareditor.cpp \
    libraryitem.cpp \
    filesystemmodel.cpp \
    shortcutlineedit.cpp \
    shortcutwidget.cpp \
    tracksnotfoundmessagebox.cpp

HEADERS += \
    mainwindow.h \
    circleprogressbar.h \
    libraryfilterlineedit.h \
    libraryfilterproxymodel.h \
    librarymodel.h \
    librarytreeview.h \
    mediabutton.h \
    musicsearchengine.h \
    playlist.h \
    settings.h \
    tabplaylist.h \
    customizethemedialog.h \
    customizeoptionsdialog.h \
    libraryitemdelegate.h \
    starrating.h \
    stareditor.h \
    libraryitem.h \
    filesystemmodel.h \
    shortcutlineedit.h \
    shortcutwidget.h \
    tracksnotfoundmessagebox.h

TRANSLATIONS = translations/m4p_ar.ts \
    translations/m4p_de.ts \
    translations/m4p_en.ts \
    translations/m4p_es.ts \
    translations/m4p_fr.ts \
    translations/m4p_ja.ts \
    translations/m4p_pt.ts \
    translations/m4p_ru.ts \
    translations/m4p_zh.ts

RESOURCES += \
    mmmmp.qrc

CONFIG(debug, debug|release) {
    LIBS += -Ldebug -ltaglib
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    #UI_DIR += debug/.ui
}

CONFIG(release, debug|release) {
    LIBS += -Lrelease -ltaglib
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    #UI_DIR += release/.ui
}

HEADERS += $$TagLibDirectory/include/taglib/audioproperties.h \
    $$TagLibDirectory/include/ape/apefile.h \
    $$TagLibDirectory/include/ape/apeproperties.h \
    $$TagLibDirectory/include/flac/flacunknownmetadatablock.h \
    $$TagLibDirectory/include/flac/flacmetadatablock.h \
    $$TagLibDirectory/include/flac/flacpicture.h \
    $$TagLibDirectory/include/fileref.h \
    $$TagLibDirectory/include/tag.h \
    $$TagLibDirectory/include/taglib_export.h \
    $$TagLibDirectory/include/tagunion.h \
    $$TagLibDirectory/include/ape/apefooter.h \
    $$TagLibDirectory/include/ape/apeitem.h \
    $$TagLibDirectory/include/ape/apetag.h \
    $$TagLibDirectory/include/asf/asfattribute.h \
    $$TagLibDirectory/include/asf/asffile.h \
    $$TagLibDirectory/include/asf/asfproperties.h \
    $$TagLibDirectory/include/asf/asftag.h \
    $$TagLibDirectory/include/flac/flacfile.h \
    $$TagLibDirectory/include/flac/flacproperties.h \
    $$TagLibDirectory/include/mp4/mp4atom.h \
    $$TagLibDirectory/include/mp4/mp4coverart.h \
    $$TagLibDirectory/include/mp4/mp4file.h \
    $$TagLibDirectory/include/mp4/mp4item.h \
    $$TagLibDirectory/include/mp4/mp4properties.h \
    $$TagLibDirectory/include/mp4/mp4tag.h \
    $$TagLibDirectory/include/mpc/mpcfile.h \
    $$TagLibDirectory/include/mpc/mpcproperties.h \
    $$TagLibDirectory/include/mpeg/mpegfile.h \
    $$TagLibDirectory/include/mpeg/mpegheader.h \
    $$TagLibDirectory/include/mpeg/mpegproperties.h \
    $$TagLibDirectory/include/mpeg/xingheader.h \
    $$TagLibDirectory/include/mpeg/id3v1/id3v1genres.h \
    $$TagLibDirectory/include/mpeg/id3v1/id3v1tag.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2extendedheader.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2footer.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2frame.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2framefactory.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2header.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2synchdata.h \
    $$TagLibDirectory/include/mpeg/id3v2/id3v2tag.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/attachedpictureframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/commentsframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/generalencapsulatedobjectframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/popularimeterframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/privateframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/relativevolumeframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/textidentificationframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/uniquefileidentifierframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/unknownframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/unsynchronizedlyricsframe.h \
    $$TagLibDirectory/include/mpeg/id3v2/frames/urllinkframe.h \
    $$TagLibDirectory/include/ogg/oggfile.h \
    $$TagLibDirectory/include/ogg/oggpage.h \
    $$TagLibDirectory/include/ogg/oggpageheader.h \
    $$TagLibDirectory/include/ogg/xiphcomment.h \
    $$TagLibDirectory/include/ogg/flac/oggflacfile.h \
    $$TagLibDirectory/include/ogg/speex/speexfile.h \
    $$TagLibDirectory/include/ogg/speex/speexproperties.h \
    $$TagLibDirectory/include/ogg/vorbis/vorbisfile.h \
    $$TagLibDirectory/include/ogg/vorbis/vorbisproperties.h \
    $$TagLibDirectory/include/riff/rifffile.h \
    $$TagLibDirectory/include/riff/aiff/aifffile.h \
    $$TagLibDirectory/include/riff/aiff/aiffproperties.h \
    $$TagLibDirectory/include/riff/wav/wavfile.h \
    $$TagLibDirectory/include/riff/wav/wavproperties.h \
    $$TagLibDirectory/include/toolkit/taglib.h \
    $$TagLibDirectory/include/toolkit/tbytevector.h \
    $$TagLibDirectory/include/toolkit/tbytevectorlist.h \
    $$TagLibDirectory/include/toolkit/tdebug.h \
    $$TagLibDirectory/include/toolkit/tfile.h \
    $$TagLibDirectory/include/toolkit/tlist.h \
    $$TagLibDirectory/include/toolkit/tmap.h \
    $$TagLibDirectory/include/toolkit/tstring.h \
    $$TagLibDirectory/include/toolkit/tstringlist.h \
    $$TagLibDirectory/include/toolkit/unicode.h \
    $$TagLibDirectory/include/trueaudio/trueaudiofile.h \
    $$TagLibDirectory/include/trueaudio/trueaudioproperties.h \
    $$TagLibDirectory/include/wavpack/wavpackfile.h \
    $$TagLibDirectory/include/wavpack/wavpackproperties.h

INCLUDEPATH += $$TagLibDirectory \
    $$TagLibDirectory/include \
    $$TagLibDirectory/taglib \
    $$TagLibDirectory/taglib/ape \
    $$TagLibDirectory/taglib/asf \
    $$TagLibDirectory/taglib/flac \
    $$TagLibDirectory/taglib/mp4 \
    $$TagLibDirectory/taglib/mpc \
    $$TagLibDirectory/taglib/mpeg \
    $$TagLibDirectory/taglib/ogg \
    $$TagLibDirectory/taglib/riff \
    $$TagLibDirectory/taglib/toolkit \
    $$TagLibDirectory/taglib/trueaudio \
    $$TagLibDirectory/taglib/wavpack \
    $$TagLibDirectory/taglib/mpeg/id3v1 \
    $$TagLibDirectory/taglib/mpeg/id3v2 \
    $$TagLibDirectory/taglib/mpeg/id3v2/frames \
    $$TagLibDirectory/taglib/ogg/flac \
    $$TagLibDirectory/taglib/ogg/speex \
    $$TagLibDirectory/taglib/ogg/vorbis \
    $$TagLibDirectory/taglib/riff/aiff \
    $$TagLibDirectory/taglib/riff/wav

OTHER_FILES +=
