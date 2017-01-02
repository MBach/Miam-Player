TEMPLATE = subdirs

win32 {
    exists(windows-toolbar/windows-toolbar.pro) {
        message( "Adding windows-toolbar plugin into the build." )
        SUBDIRS += windows-toolbar
    }
    exists(Miam-Player-shell/miam-player-shell.pro) {
        message( "Adding Miam-Player-shell plugin into the build." )
        SUBDIRS += Miam-Player-shell
    }
}

exists(acoustid-plugin/acoustid-plugin.pro) {
    message( "Adding acoustid-plugin plugin into the build." )
    SUBDIRS += acoustid-plugin
}

exists(cover-fetcher/cover-fetcher.pro) {
    message( "Adding cover-fetcher plugin into the build." )
    SUBDIRS += cover-fetcher
}

exists(deezer-plugin/deezer-plugin.pro) {
    message( "Adding deezer-plugin plugin into the build." )
    SUBDIRS += deezer-plugin
}

exists(soundcloud-plugin/soundcloud-plugin.pro) {
    message( "Adding soundcloud-plugin plugin into the build." )
    SUBDIRS += soundcloud-plugin
}

RESOURCES += 
