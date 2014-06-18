This directory and its subdirectories are not complete here. Mostly to keep the repository light because .lib and .dll are already included somewhere else in this git repository.

The folder "data" in each subdirectory is missing. It will be created by the script "create_package_x64.bat". However, everything is not fully automated. You have to edit this script and modify hardcoded path.

List of variables to modify:

set MiamPlayer-build="C:\dev\Miam-Player-build-x64"
set MiamPlayer-libs="C:\dev\Miam-Player\lib\release\win-x64"
set QTDIR="C:\Qt\Qt5.3.0\5.3\msvc2013_64"
set CoverFetcher-build="C:\dev\cover-fetcher-build-x64\release"
set MiniMode-build="C:\dev\mini-mode-build-x64\release"
set WindowsToolbar-build="C:\dev\windows-toolbar-build-x64\release"

That's it! You have nothing else to do.
