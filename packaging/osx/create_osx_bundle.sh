#!/bin/bash

libDir=../../lib/osx

# Where is installed Qt?
qtDir=~/Qt5.4.1/

# Where is the player built with QtCreator?
build=~/dev/Miam-Player-build

# Where is installed VLC?
vlcDir=/Applications/VLC.app

# Current release
version=0.7.1

### Libraries
# Modify uncomplete App first
cp -R ${build}/MiamPlayer/MiamPlayer.app .
frameworks=MiamPlayer.app/Contents/Frameworks
mkdir -p ${frameworks}
mkdir -p MiamPlayer.app/Contents/PlugIns

# 3rd party dependencies (TagLib, VLC-Qt)
cp ${libDir}/*.dylib ${frameworks}

# Copy VLC libraries
cp ${vlcDir}/Contents/MacOS/lib/liblzma.5.dylib ${frameworks}
cp ${vlcDir}/Contents/MacOS/lib/libvlccore.8.dylib ${frameworks}
cp ${vlcDir}/Contents/MacOS/lib/libvlc.5.dylib ${frameworks}

install_name_tool -id "@executable_path/../Frameworks/liblzma.5.dylib" ${frameworks}/liblzma.5.dylib
install_name_tool -id "@executable_path/../Frameworks/libvlccore.8.dylib" ${frameworks}/libvlccore.8.dylib

# Copy VLC plugins (loaded at runtime)
cp ${vlcDir}/Contents/MacOS/plugins/* MiamPlayer.app/Contents/PlugIns/
for plugin in MiamPlayer.app/Contents/PlugIns/*.dylib
do
    install_name_tool -change "@loader_path/../lib/libvlccore.8.dylib" "@loader_path/../Frameworks/libvlccore.8.dylib" ${plugin};
done

# Own dependencies
cp ${build}/MiamCore/libmiam-core.*.dylib ${frameworks}
cp ${build}/MiamUniqueLibrary/libmiam-uniquelibrary.*.dylib ${frameworks}

### Bundle
cp Info.plist MiamPlayer.app/Contents/
cp MiamPlayer.icns MiamPlayer.app/Contents/Resources/

# Create bundle
${qtDir}/5.4/clang_64/bin/macdeployqt MiamPlayer.app -always-overwrite

# Wtf? If set before macdeployqt it's not written, unlike first 2 calls of install_name_tool
install_name_tool -change "@loader_path/../lib/libvlccore.8.dylib" "@executable_path/../Frameworks/libvlccore.8.dylib" ${frameworks}/libvlc.5.dylib

# Create the final redistributable package
rm -rf MiamPlayer-${version}.dmg
appdmg spec.json MiamPlayer-${version}.dmg > /dev/null 2>&1 || {
    echo >&2 "appdmg is required to build the final package. Please install homebrew and appdmg. Aborting.";
    exit 1;
}
