#!/bin/bash

cd "./plugins/"
for file in *
do
    echo $file
    install_name_tool -change "@loader_path/../lib/libvlccore.8.dylib" "@executable_path/../Frameworks/libvlccore.8.dylib" $file;
done
