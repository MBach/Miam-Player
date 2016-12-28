TEMPLATE = subdirs

CONFIG += ordered warn_on qt debug_and_release

SUBDIRS += \
    src/Core \
    src/Library \
    src/TabPlaylists \
    src/UniqueLibrary \
    src/Player

RESOURCES += \
    src/Player/mp.qrc \
    src/TabPlaylists/mp.qrc
