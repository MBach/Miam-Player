#!/bin/bash

# (No need to be changed)
libDir=../lib/osx

# /!\ Where is installed Qt?
qtDir=/Users/mbach/Qt5.6.0/5.6/clang_64

# /!\ Where is the player built with QtCreator?
build=/Users/mbach/dev/Miam-Player-build/src

# /!\ Current release
version=0.8.0

# Everything else below should be straight forward

### Libraries
# Copy and modify uncomplete App first
cp -R ${build}/MiamPlayer/MiamPlayer.app .
frameworks=MiamPlayer.app/Contents/Frameworks
mkdir -p ${frameworks}

# 3rd party dependencies (TagLib, QtAV)
cp ${libDir}/*.dylib ${frameworks}
cp -R ${libDir}/QtAV.framework ${frameworks}/QtAV.framework

# Change how MiamCore is linked to QtAV (removing absolute paths)
for core in MiamPlayer.app/Contents/Frameworks/libmiam-core.*.dylib
do
    install_name_tool -change "${qtDir}/lib//QtAV.framework/Versions/1/QtAV" "@rpath/QtAV.framework/Versions/1/QtAV" ${core};
done

### Bundle
cp Info.plist MiamPlayer.app/Contents/

# Create bundle
${qtDir}/bin/macdeployqt MiamPlayer.app -always-overwrite -appstore-compliant

# Somehow, macdeployqt messes around with other packages
# Change how QtAV is linked to Qt Framework
install_name_tool -id "@rpath/QtAV.framework/Versions/1/QtAV" ${frameworks}/QtAV.framework/QtAV
install_name_tool -change "${qtDir}/lib//QtGui.framework/Versions/5/QtGui" "@rpath/QtGui.framework/Versions/5/QtGui" ${frameworks}/QtAV.framework/QtAV
install_name_tool -change "${qtDir}/lib//QtCore.framework/Versions/5/QtCore" "@rpath/QtCore.framework/Versions/5/QtCore" ${frameworks}/QtAV.framework/QtAV

# Create the final redistributable package
rm -rf MiamPlayer-${version}.dmg
appdmg spec.json MiamPlayer-${version}.dmg > /dev/null 2>&1 || {
    echo >&2 "appdmg is required to build the final package. Please install npm and appdmg. Aborting.";
    exit 1;
}
