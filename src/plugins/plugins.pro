TEMPLATE = subdirs

exists(windows-toolbar) {
    message( "Adding windows-toolbar plugin into the build." )
    SUBDIRS += windows-toolbar
}

exists(acoustid-plugin) {
    message( "Adding acoustid-plugin plugin into the build." )
    SUBDIRS += acoustid-plugin
}

RESOURCES += 
