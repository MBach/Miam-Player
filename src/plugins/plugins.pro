TEMPLATE = subdirs

win32 {
    exists(windows-toolbar/windows-toolbar.pro) {
        message( "windows-toolbar plugin added to the build." )
        SUBDIRS += windows-toolbar
    }
    exists(Miam-Player-shell/miam-player-shell.pro) {
        message( "Miam-Player-shell plugin added to the build." )
        SUBDIRS += Miam-Player-shell
    }
}

exists(acoustid-plugin/acoustid-plugin.pro) {
    message( "acoustid-plugin added to the build." )
    SUBDIRS += acoustid-plugin
}

exists(cover-fetcher/cover-fetcher.pro) {
    message( "cover-fetcher plugin added to the build." )
    SUBDIRS += cover-fetcher
}

exists(deezer-plugin/deezer-plugin.pro) {
    message( "deezer-plugin added to the build." )
    SUBDIRS += deezer-plugin
}

exists(soundcloud-plugin/soundcloud-plugin.pro) {
    message( "soundcloud-plugin added to the build." )
    SUBDIRS += soundcloud-plugin
}
