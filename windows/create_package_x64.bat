@echo off

rem core
set MiamPlayerBuild="C:\dev\Miam-Player-build\src"
mkdir packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\MiamCore\release\MiamCore.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\MiamLibrary\release\MiamLibrary.dll packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\MiamPlayer\release\MiamPlayer.exe packages\org.miamplayer.core\data\
copy %MiamPlayerBuild%\MiamUniqueLibrary\release\MiamUniqueLibrary.dll packages\org.miamplayer.core\data\

rem 3rd party
set MiamPlayerLibs="C:\dev\Miam-Player\lib\release\win-x64"
mkdir packages\org.qtav\data\
copy %MiamPlayerLibs%\*.dll packages\org.qtav\data\

rem qt libraries
set QTDIR="C:\Qt\Qt5.6.0\5.6\msvc2015_64"
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
copy %QTDIR%\bin\icudt54.dll packages\org.miamplayer.core\data\icudt54.dll
copy %QTDIR%\bin\icuin54.dll packages\org.miamplayer.core\data\icuin54.dll
copy %QTDIR%\bin\icuuc54.dll packages\org.miamplayer.core\data\icuuc54.dll
copy %QTDIR%\bin\libEGL.dll packages\org.miamplayer.core\data\libEGL.dll
copy %QTDIR%\bin\libGLESv2.dll packages\org.miamplayer.core\data\libGLESv2.dll
copy %QTDIR%\bin\Qt5Core.dll packages\org.miamplayer.core\data\Qt5Core.dll
copy %QTDIR%\bin\Qt5Gui.dll packages\org.miamplayer.core\data\Qt5Gui.dll
copy %QTDIR%\bin\Qt5Multimedia.dll packages\org.miamplayer.core\data\Qt5Multimedia.dll
copy %QTDIR%\bin\Qt5MultimediaWidgets.dll packages\org.miamplayer.core\data\Qt5MultimediaWidgets.dll
copy %QTDIR%\bin\Qt5Network.dll packages\org.miamplayer.core\data\Qt5Network.dll
copy %QTDIR%\bin\Qt5OpenGL.dll packages\org.miamplayer.core\data\Qt5OpenGL.dll
copy %QTDIR%\bin\Qt5Sql.dll packages\org.miamplayer.core\data\Qt5Sql.dll
copy %QTDIR%\bin\Qt5Widgets.dll packages\org.miamplayer.core\data\Qt5Widgets.dll
copy %QTDIR%\bin\Qt5WinExtras.dll packages\org.miamplayer.core\data\Qt5WinExtras.dll

rem official plugins
set AcoustIDSRC="C:\dev\acoustid-plugin\lib\release"
set AcoustIDBuild="C:\dev\acoustid-plugin-build\release"
set CoverFetcherBuild="C:\dev\cover-fetcher-build\release"
set DeezerPluginBuild="C:\dev\deezer-plugin-build\release"
set MiamPlayerShellBuild="C:\dev\Miam-Player-shell-build"
set MiniModeBuild="C:\dev\mini-mode-build\release"
set WindowsToolbarBuild="C:\dev\windows-toolbar-build\release"

xcopy %AcoustIDBuild%\acoustid-plugin.dll packages\org.miamplayer.plugins.acoustid\data\plugins\ /y/e
rem Chromaprint dependency isn't a plugin itself so it must be put next to FFMpeg
copy %AcoustIDSRC%\chromaprint.dll packages\org.miamplayer.plugins.acoustid\data\chromaprint.dll
xcopy %CoverFetcherBuild%\cover-fetcher.dll packages\org.miamplayer.plugins.coverfetcher\data\plugins\ /y/e
xcopy %MiamPlayerShellBuild%\MiamShell\release\MiamPlayerShell.dll packages\org.miamplayer.plugins.miamplayershell\data\ /y/e
xcopy %MiamPlayerShellBuild%\MiamShellGui\release\MiamShellGui.dll packages\org.miamplayer.plugins.miamplayershell\data\plugins\ /y/e
xcopy %MiniModeBuild%\mini-mode.dll packages\org.miamplayer.plugins.minimode\data\plugins\ /y/e
xcopy %WindowsToolbarBuild%\windows-toolbar.dll packages\org.miamplayer.plugins.windowstoolbar\data\plugins\ /y/e

rem Deezer talks to the Internet with QtWebKit
xcopy %DeezerPluginBuild%\deezer-plugin.dll packages\org.miamplayer.plugins.deezer\data\plugins\ /y/e
copy %QTDIR%\bin\Qt5WebEngine.dll packages\org.miamplayer.plugins.deezer\data\Qt5WebEngine.dll
copy %QTDIR%\bin\Qt5WebEngineCore.dll packages\org.miamplayer.plugins.deezer\data\Qt5WebEngineCore.dll

rem WebKit has a lot of dependencies!
rem copy %QTDIR%\bin\Qt5Positioning.dll packages\org.miamplayer.plugins.deezer\data\Qt5Positioning.dll
rem copy %QTDIR%\bin\Qt5PrintSupport.dll packages\org.miamplayer.plugins.deezer\data\Qt5PrintSupport.dll
rem copy %QTDIR%\bin\Qt5Qml.dll packages\org.miamplayer.plugins.deezer\data\Qt5Qml.dll
rem copy %QTDIR%\bin\Qt5Quick.dll packages\org.miamplayer.plugins.deezer\data\Qt5Quick.dll
rem copy %QTDIR%\bin\Qt5Sensors.dll packages\org.miamplayer.plugins.deezer\data\Qt5Sensors.dll
rem copy %QTDIR%\bin\Qt5WebChannel.dll packages\org.miamplayer.plugins.deezer\data\Qt5WebChannel.dll

rem create only repository
rem rmdir repository /s /q
rem repogen -p packages repository

rem create the final package
binarycreator --offline-only -c config/config.xml -r resources/additional.qrc -p packages MiamPlayer-0.8.0.exe

rem delete data folders
rmdir packages\org.qtav\data\ /s /q
rmdir packages\org.miamplayer.core\data\ /s /q
rmdir packages\org.miamplayer.plugins.acoustid\data\ /s /q
rmdir packages\org.miamplayer.plugins.coverfetcher\data\ /s /q
rmdir packages\org.miamplayer.plugins.deezer\data\ /s /q
rmdir packages\org.miamplayer.plugins.miamplayershell\data\ /s /q
rmdir packages\org.miamplayer.plugins.minimode\data\ /s /q
rmdir packages\org.miamplayer.plugins.windowstoolbar\data\ /s /q
