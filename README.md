# Miam-Player
[![Homepage][web-img]][web]
[![Latest release][release-img]][release]
[![License][license-img]][license]

Miam-Player is a cross-platform open source music player
![general overview](http://www.miam-player.org/images/gallery/general_overview.png)

# What's inside?
* Fast and reliable
* Read .mp3, .m4a (MP4), .flac, .ogg, .oga (OGG Vorbis), .asf, .ape (Monkey Audio), .opus and more!
* Customize everything: User Interface, Covers, Shortcuts, Buttons, Themes
* Read and edit metadata using [Taglib](http://taglib.github.io/)

# Future work
* 10-band equalizer
* Add [projectM](http://projectm.sourceforge.net/) support for amazing visualizations
* A spotify / jamendo plugin to stream music
![equalizer](http://miam-player.org/images/work-in-progress/equalizer.png)
* Useless: (scriptable) bitorrent client where you could browse results from your favorite tracker

# Plugins
A plugin system has been introduced to extend possibilites of the player. Five official plugins come with the installer for Windows:
* Support thumbnails buttons to control the player, green progress bar when playing, overlay icon
* A mini-mode which is a tribute to the good ol' Winamp _shade mode_: ![mini](http://www.miam-player.org/images/gallery/mini-mode.png)
* Get missing covers by looking at [MusicBrainz'](https://musicbrainz.org/) repository
* Shell explorer extension (up to 4 actions, like 'Send to current playlist')
* Deezer plugin (technology preview): merge local and remote Artists / Albums: ![mini](http://miam-player.org/images/work-in-progress/deezer.png)

## Binaries and source code

You can download Miam-Player binaries from the [official website][web]

Latest source is also available via Git:
```bash
  git clone git://github.com/MBach/Miam-Player.git
```

Master branch is _not always_ stable but should work on all supported platforms. You can find specific working releases by browsing tags.


## Requirements

Miam-Player can be built with any common compiler (g++, clang, MSVC, MinGW).
Build files are generated using [QMake](http://doc.qt.io/qt-5/qmake-manual.html).

If you feel brave to write a [CMake](http://www.cmake.org/) configuration, please contribute!

You need to build dependencies first: QtAV and TagLib. Pre-built libraries are provided for Windows and OSX in this repository.

See the [wiki](http://www.miam-player.org/wiki/index.php?title=How-to-build-Miam-Player) for more details.

## Copyright info

Copyright (C) 2012-2015 Matthieu Bachelier

Miam-Player is free (libre) software. This means the complete player
source code is available to public, anyone is welcome to research
how the application works, participate in its development, freely
distribute the application and spread the word!

This project may be used under the terms of the
GNU General Public License version 3.0 as published by the
Free Software Foundation and appearing in the file LICENSE


[web]: http://www.miam-player.org
[release]: https://github.com/MBach/Miam-Player/releases
[license]: https://github.com/MBach/Miam-Player/blob/master/LICENSE

[web-img]: https://img.shields.io/badge/web-miam--player.org-green.svg
[license-img]: https://img.shields.io/github/license/MBach/miam-player.svg
[release-img]: https://img.shields.io/github/release/MBach/miam-player.svg
