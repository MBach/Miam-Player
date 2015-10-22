##This folder contains pre-compiled libraries

You will find the following libraries [TagLib](http://taglib.github.io/) and [QtAV](http://www.qtav.org/).
I know these files shouldn't belong here. But it keeps things way more simple to have some binaries precompiled. They are automatically recognized by the *.pro file and QtCreator when building the project so you can press the green arrow Play, wait for a while, and launch the player.

These files are used for compiling subproject `MiamCore`. You can also rebuild these files for Windows/OSX if you wish.

###Folders *debug* and *release* (x64 only)
- Files for Windows. Built with MSVC2013

###Folder *osx*
- Contains only `libtag.dylib`

> So? There are no pre-compiled files for Unix/Linux?
>
> No, you have to find or build them manually. 
