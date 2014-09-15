This directory and its subdirectories are not complete here. Mostly to keep the repository light because .lib and .dll are already included somewhere else in this git repository.

The folder "data" in each subdirectory is missing. It will be created by the script "create_package_x64.bat". However, everything is not fully automated. You have to edit this script and modify hardcoded path.

List of variables to modify:

set MiamPlayerBuild="C:\dev\Miam-Player-build-x64"
set MiamPlayerLibs="C:\dev\Miam-Player\lib\release\win-x64"
set QTDIR="C:\Qt\Qt5.3.0\5.3\msvc2013_64"
set CoverFetcherBuild="C:\dev\cover-fetcher-build-x64\release"
set MiniModeBuild="C:\dev\mini-mode-build-x64\release"
set WindowsToolbarBuild="C:\dev\windows-toolbar-build-x64\release"

That's it! You have nothing else to do.
