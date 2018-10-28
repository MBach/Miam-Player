@echo off

rem core
set MiamPlayerBuild="C:\dev\build-miam-player-Desktop_Qt_5_11_1_MSVC2017_64bit-Release\src"
mkdir packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\acoustid\release\miam-acoustid.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\core\release\miam-core.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\cover-fetcher\release\miam-coverfetcher.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\library\release\miam-library.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\player\release\MiamPlayer.exe packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\tabplaylists\release\miam-tabplaylists.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\tageditor\release\miam-tageditor.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\uniquelibrary\release\miam-uniquelibrary.dll packages\org.miamplayer.core\data\

rem 3rd party
set MiamPlayerLibs="C:\dev\Miam-Player\lib\release\win-x64"
mkdir packages\org.qtav\data\
copy %MiamPlayerLibs%\*.dll packages\org.qtav\data\

rem qt libraries
set QTDIR="C:\Qt\5.11.1\msvc2017_64"
echo f | xcopy %QTDIR%\plugins\bearer\qgenericbearer.dll packages\org.miamplayer.core\data\bearer\qgenericbearer.dll /y/s
echo f | xcopy %QTDIR%\plugins\imageformats\qjpeg.dll packages\org.miamplayer.core\data\imageformats\qjpeg.dll /y/s
echo f | xcopy %QTDIR%\plugins\mediaservice\qtmedia_audioengine.dll packages\org.miamplayer.core\data\mediaservice\qtmedia_audioengine.dll /y/s
echo f | xcopy %QTDIR%\plugins\mediaservice\wmfengine.dll packages\org.miamplayer.core\data\mediaservice\wmfengine.dll /y/s
echo f | xcopy %QTDIR%\plugins\platforms\qminimal.dll packages\org.miamplayer.core\data\platforms\qminimal.dll /y/s
echo f | xcopy %QTDIR%\plugins\platforms\qwindows.dll packages\org.miamplayer.core\data\platforms\qwindows.dll /y/s
echo f | xcopy %QTDIR%\plugins\playlistformats\qtmultimedia_m3u.dll packages\org.miamplayer.core\data\playlistformats\qtmultimedia_m3u.dll /y/s
echo f | xcopy %QTDIR%\plugins\sqldrivers\qsqlite.dll packages\org.miamplayer.core\data\sqldrivers\qsqlite.dll /y/s
echo f | xcopy %QTDIR%\translations\qt_ar.qm packages\org.miamplayer.core\data\translations\qt_ar.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_cs.qm packages\org.miamplayer.core\data\translations\qt_cs.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_de.qm packages\org.miamplayer.core\data\translations\qt_de.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_es.qm packages\org.miamplayer.core\data\translations\qt_es.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_fr.qm packages\org.miamplayer.core\data\translations\qt_fr.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_it.qm packages\org.miamplayer.core\data\translations\qt_it.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_ja.qm packages\org.miamplayer.core\data\translations\qt_ja.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_ko.qm packages\org.miamplayer.core\data\translations\qt_ko.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_pt.qm packages\org.miamplayer.core\data\translations\qt_pt.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_ru.qm packages\org.miamplayer.core\data\translations\qt_ru.qm /y/s
echo f | xcopy %QTDIR%\translations\qt_zh_CN.qm packages\org.miamplayer.core\data\translations\qt_zh_CN.qm /y/s
copy %QTDIR%\bin\libEGL.dll packages\org.miamplayer.core\data\libEGL.dll
copy %QTDIR%\bin\libGLESv2.dll packages\org.miamplayer.core\data\libGLESv2.dll
copy %QTDIR%\bin\Qt5Core.dll packages\org.miamplayer.core\data\Qt5Core.dll
copy %QTDIR%\bin\Qt5Gui.dll packages\org.miamplayer.core\data\Qt5Gui.dll
copy %QTDIR%\bin\Qt5Multimedia.dll packages\org.miamplayer.core\data\Qt5Multimedia.dll
copy %QTDIR%\bin\Qt5MultimediaWidgets.dll packages\org.miamplayer.core\data\Qt5MultimediaWidgets.dll
copy %QTDIR%\bin\Qt5Network.dll packages\org.miamplayer.core\data\Qt5Network.dll
copy %QTDIR%\bin\Qt5OpenGL.dll packages\org.miamplayer.core\data\Qt5OpenGL.dll
copy %QTDIR%\bin\Qt5Sql.dll packages\org.miamplayer.core\data\Qt5Sql.dll
copy %QTDIR%\bin\Qt5WebSockets.dll packages\org.miamplayer.core\data\Qt5WebSockets.dll
copy %QTDIR%\bin\Qt5Widgets.dll packages\org.miamplayer.core\data\Qt5Widgets.dll
copy %QTDIR%\bin\Qt5WinExtras.dll packages\org.miamplayer.core\data\Qt5WinExtras.dll

rem official plugins
rem set DeezerPluginBuild="C:\dev\deezer-plugin-build\release"
rem set MiamPlayerShellBuild="C:\dev\Miam-Player-shell-build"

rem FIXME
rem xcopy %MiamPlayerShellBuild%\MiamShell\release\MiamPlayerShell.dll packages\org.miamplayer.plugins.miamplayershell\data\ /y/e
rem xcopy %MiamPlayerShellBuild%\MiamShellGui\release\MiamShellGui.dll packages\org.miamplayer.plugins.miamplayershell\data\plugins\ /y/e
xcopy %MiamPlayerBuild%\player\release\plugins\windows-toolbar.dll packages\org.miamplayer.plugins.windowstoolbar\data\plugins\ /y/e

rem Deezer talks to the Internet with QtWebKit
rem xcopy %DeezerPluginBuild%\deezer-plugin.dll packages\org.miamplayer.plugins.deezer\data\plugins\ /y/e
rem copy %QTDIR%\bin\Qt5WebEngine.dll packages\org.miamplayer.plugins.deezer\data\Qt5WebEngine.dll
rem copy %QTDIR%\bin\Qt5WebEngineCore.dll packages\org.miamplayer.plugins.deezer\data\Qt5WebEngineCore.dll

rem create the final package
binarycreator --offline-only -c config/config.xml -r resources/additional.qrc -p packages MiamPlayer-0.9.0.exe

rem delete data folders
rmdir packages\org.qtav\data\ /s /q
rmdir packages\org.miamplayer.core\data\ /s /q
rem rmdir packages\org.miamplayer.plugins.deezer\data\ /s /q
rem rmdir packages\org.miamplayer.plugins.miamplayershell\data\ /s /q
rmdir packages\org.miamplayer.plugins.windowstoolbar\data\ /s /q
