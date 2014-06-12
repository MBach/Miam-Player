This directory and its subdirectories are not complete here. Mostly to keep the repository light because .lib and .dll are already included somewhere else in this git repository.

The folder "data" in each subdirectory is missing.

To build the final package, first copy/paste some dll which are located on your hard drive :

1) to put in .\packages\org.miamplayer.core\data\

A) find some .dll located on your hard drive:

Create folder "imageformats" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\imageformats\), add "qjpeg.dll"
Create folder "mediaservice" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\mediaservice\), add "qtmedia_audioengine.dll", "wmfengine.dll"
Create folder "platforms" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\platforms\), add "qminimal.dll"
Create folder "playlistformats" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\playlistformats\), add "qtmultimedia_m3u.dll"
Create folder "sqldrivers" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\sqldrivers\), add "qsqlite.dll"
Create folder "translations" (C:\Qt\Qt5.3.0\5.3\msvc2013_64\plugins\translations\), add : "qt_ar.qm", "qt_cs.qm", "qt_de.qm", "qt_es.qm", "qt_fr.qm", "qt_it.qm", "qt_ja.qm", "qt_ko.qm", "qt_pt.qm", "qt_ru.qm", "qt_zh_CN.qm"

Add "icudt52.dll", "icuin52.dll", "icuuc52.dll", "libEGL.dll", "libGLESv2.dll", "Qt5Core.dll", "Qt5Gui.dll", "Qt5Multimedia.dll", "Qt5MultimediaWidgets.dll", "Qt5Network.dll", "Qt5OpenGL.dll", "Qt5Sql.dll", "Qt5Widgets.dll", "Qt5WinExtras.dll"

Which are located in C:\Qt\Qt5.3.0\5.3\msvc2013_64\bin

B) Taglib and VLC-Qt

In order to make this step straightforward, go to ../../lib/release/win-x64/. The missing .dll are there.

Still in .\packages\org.miamplayer.core\data\,

Add the following files: "tag.dll", "libvlc.dll", "libvlccore.dll", "vlc-qt-widgets.dll", vlc-qt.dll" (the .lib files are required when you're compiling)
Copy/paste the folder "plugins" (dependencies and codec for VLC)

C) Finally, the binary

Go to the directory where you have build "Miam-Player". Let's say, "C:\dev\Miam-Player-build-x64\" for example.

In .\MiamCore\release\, add "MiamCore.dll"
In .\MiamPlayer\release\, add "MiamPlayer.exe"
In .\MiamUniqueLibrary\release\, add "MiamUniqueLibrary.dll"

== End of step 1) ==

To add the plugins, you need to clone 

https://github.com/MBach/cover-fetcher
https://github.com/MBach/mini-mode
https://github.com/MBach/windows-toolbar

To build them (by adding "MiamCore.lib" in each plugin build directory, otherwise the linking part will fail) and to copy/paste the new .dll

2) in .\packages\org.miamplayer.plugins.coverfetcher\data\

Add "cover-fetcher.dll" 

3) in .\packages\org.miamplayer.plugins.minimode\data\

Add "mini-mode.dll"

4) in .\packages\org.miamplayer.plugins.windowstoolbar\data\

Add "windows-toolbar.dll"


=== Help ! ===


This is what you should have: 

+---org.miamplayer.core
¦   +---data
¦   ¦   ¦   icudt52.dll
¦   ¦   ¦   icuin52.dll
¦   ¦   ¦   icuuc52.dll
¦   ¦   ¦   libEGL.dll
¦   ¦   ¦   libGLESv2.dll
¦   ¦   ¦   libvlc.dll
¦   ¦   ¦   libvlccore.dll
¦   ¦   ¦   MiamCore.dll
¦   ¦   ¦   MiamPlayer.exe
¦   ¦   ¦   MiamUniqueLibrary.dll
¦   ¦   ¦   Qt5Core.dll
¦   ¦   ¦   Qt5Gui.dll
¦   ¦   ¦   Qt5Multimedia.dll
¦   ¦   ¦   Qt5MultimediaWidgets.dll
¦   ¦   ¦   Qt5Network.dll
¦   ¦   ¦   Qt5OpenGL.dll
¦   ¦   ¦   Qt5Sql.dll
¦   ¦   ¦   Qt5Widgets.dll
¦   ¦   ¦   Qt5WinExtras.dll
¦   ¦   ¦   tag.dll
¦   ¦   ¦   vlc-qt-widgets.dll
¦   ¦   ¦   vlc-qt.dll
¦   ¦   ¦
¦   ¦   +---imageformats
¦   ¦   ¦       qjpeg.dll
¦   ¦   ¦
¦   ¦   +---mediaservice
¦   ¦   ¦       qtmedia_audioengine.dll
¦   ¦   ¦       wmfengine.dll
¦   ¦   ¦
¦   ¦   +---platforms
¦   ¦   ¦       qminimal.dll
¦   ¦   ¦
¦   ¦   +---playlistformats
¦   ¦   ¦       qtmultimedia_m3u.dll
¦   ¦   ¦
¦   ¦   +---plugins
¦   ¦   ¦   +---access
¦   ¦   ¦   ¦       libaccess_attachment_plugin.dll
¦   ¦   ¦   ¦       libaccess_bd_plugin.dll
¦   ¦   ¦   ¦       libaccess_ftp_plugin.dll
¦   ¦   ¦   ¦       libaccess_http_plugin.dll
¦   ¦   ¦   ¦       libaccess_imem_plugin.dll
¦   ¦   ¦   ¦       libaccess_mms_plugin.dll
¦   ¦   ¦   ¦       libaccess_rar_plugin.dll
¦   ¦   ¦   ¦       libaccess_realrtsp_plugin.dll
¦   ¦   ¦   ¦       libaccess_smb_plugin.dll
¦   ¦   ¦   ¦       libaccess_tcp_plugin.dll
¦   ¦   ¦   ¦       libaccess_udp_plugin.dll
¦   ¦   ¦   ¦       libaccess_vdr_plugin.dll
¦   ¦   ¦   ¦       libcdda_plugin.dll
¦   ¦   ¦   ¦       libdshow_plugin.dll
¦   ¦   ¦   ¦       libdtv_plugin.dll
¦   ¦   ¦   ¦       libdvdnav_plugin.dll
¦   ¦   ¦   ¦       libdvdread_plugin.dll
¦   ¦   ¦   ¦       libfilesystem_plugin.dll
¦   ¦   ¦   ¦       libidummy_plugin.dll
¦   ¦   ¦   ¦       liblibbluray_plugin.dll
¦   ¦   ¦   ¦       librtp_plugin.dll
¦   ¦   ¦   ¦       libscreen_plugin.dll
¦   ¦   ¦   ¦       libsdp_plugin.dll
¦   ¦   ¦   ¦       libstream_filter_rar_plugin.dll
¦   ¦   ¦   ¦       libvcd_plugin.dll
¦   ¦   ¦   ¦       libzip_plugin.dll
¦   ¦   ¦   ¦
¦   ¦   ¦   +---audio_filter
¦   ¦   ¦   ¦       liba52tofloat32_plugin.dll
¦   ¦   ¦   ¦       liba52tospdif_plugin.dll
¦   ¦   ¦   ¦       libaudiobargraph_a_plugin.dll
¦   ¦   ¦   ¦       libaudio_format_plugin.dll
¦   ¦   ¦   ¦       libchorus_flanger_plugin.dll
¦   ¦   ¦   ¦       libcompressor_plugin.dll
¦   ¦   ¦   ¦       libconverter_fixed_plugin.dll
¦   ¦   ¦   ¦       libdolby_surround_decoder_plugin.dll
¦   ¦   ¦   ¦       libdtstofloat32_plugin.dll
¦   ¦   ¦   ¦       libdtstospdif_plugin.dll
¦   ¦   ¦   ¦       libequalizer_plugin.dll
¦   ¦   ¦   ¦       libheadphone_channel_mixer_plugin.dll
¦   ¦   ¦   ¦       libkaraoke_plugin.dll
¦   ¦   ¦   ¦       libmono_plugin.dll
¦   ¦   ¦   ¦       libmpgatofixed32_plugin.dll
¦   ¦   ¦   ¦       libnormvol_plugin.dll
¦   ¦   ¦   ¦       libparam_eq_plugin.dll
¦   ¦   ¦   ¦       libsamplerate_plugin.dll
¦   ¦   ¦   ¦       libscaletempo_plugin.dll
¦   ¦   ¦   ¦       libsimple_channel_mixer_plugin.dll
¦   ¦   ¦   ¦       libspatializer_plugin.dll
¦   ¦   ¦   ¦       libspeex_resampler_plugin.dll
¦   ¦   ¦   ¦       libtrivial_channel_mixer_plugin.dll
¦   ¦   ¦   ¦       libugly_resampler_plugin.dll
¦   ¦   ¦   ¦
¦   ¦   ¦   +---audio_output
¦   ¦   ¦   ¦       libadummy_plugin.dll
¦   ¦   ¦   ¦       libamem_plugin.dll
¦   ¦   ¦   ¦       libaout_directx_plugin.dll
¦   ¦   ¦   ¦       libaout_file_plugin.dll
¦   ¦   ¦   ¦       libwaveout_plugin.dll
¦   ¦   ¦   ¦
¦   ¦   ¦   +---codec
¦   ¦   ¦   ¦       liba52_plugin.dll
¦   ¦   ¦   ¦       libadpcm_plugin.dll
¦   ¦   ¦   ¦       libaes3_plugin.dll
¦   ¦   ¦   ¦       libaraw_plugin.dll
¦   ¦   ¦   ¦       libavcodec_plugin.dll
¦   ¦   ¦   ¦       libcc_plugin.dll
¦   ¦   ¦   ¦       libcdg_plugin.dll
¦   ¦   ¦   ¦       libcvdsub_plugin.dll
¦   ¦   ¦   ¦       libddummy_plugin.dll
¦   ¦   ¦   ¦       libdmo_plugin.dll
¦   ¦   ¦   ¦       libdts_plugin.dll
¦   ¦   ¦   ¦       libdvbsub_plugin.dll
¦   ¦   ¦   ¦       libedummy_plugin.dll
¦   ¦   ¦   ¦       libfaad_plugin.dll
¦   ¦   ¦   ¦       libflac_plugin.dll
¦   ¦   ¦   ¦       libfluidsynth_plugin.dll
¦   ¦   ¦   ¦       libkate_plugin.dll
¦   ¦   ¦   ¦       liblibass_plugin.dll
¦   ¦   ¦   ¦       liblibmpeg2_plugin.dll
¦   ¦   ¦   ¦       liblpcm_plugin.dll
¦   ¦   ¦   ¦       libmpeg_audio_plugin.dll
¦   ¦   ¦   ¦       libopus_plugin.dll
¦   ¦   ¦   ¦       libpng_plugin.dll
¦   ¦   ¦   ¦       libquicktime_plugin.dll
¦   ¦   ¦   ¦       librawvideo_plugin.dll
¦   ¦   ¦   ¦       libschroedinger_plugin.dll
¦   ¦   ¦   ¦       libspeex_plugin.dll
¦   ¦   ¦   ¦       libspudec_plugin.dll
¦   ¦   ¦   ¦       libstl_plugin.dll
¦   ¦   ¦   ¦       libsubsdec_plugin.dll
¦   ¦   ¦   ¦       libsubsusf_plugin.dll
¦   ¦   ¦   ¦       libsvcdsub_plugin.dll
¦   ¦   ¦   ¦       libt140_plugin.dll
¦   ¦   ¦   ¦       libtheora_plugin.dll
¦   ¦   ¦   ¦       libtwolame_plugin.dll
¦   ¦   ¦   ¦       libvorbis_plugin.dll
¦   ¦   ¦   ¦       libx264_plugin.dll
¦   ¦   ¦   ¦       libzvbi_plugin.dll
¦   ¦   ¦   ¦
¦   ¦   ¦   +---demux
¦   ¦   ¦   ¦       libaiff_plugin.dll
¦   ¦   ¦   ¦       libasf_plugin.dll
¦   ¦   ¦   ¦       libau_plugin.dll
¦   ¦   ¦   ¦       libavi_plugin.dll
¦   ¦   ¦   ¦       libdemuxdump_plugin.dll
¦   ¦   ¦   ¦       libdemux_cdg_plugin.dll
¦   ¦   ¦   ¦       libdemux_stl_plugin.dll
¦   ¦   ¦   ¦       libdirac_plugin.dll
¦   ¦   ¦   ¦       libes_plugin.dll
¦   ¦   ¦   ¦       libflacsys_plugin.dll
¦   ¦   ¦   ¦       libgme_plugin.dll
¦   ¦   ¦   ¦       libh264_plugin.dll
¦   ¦   ¦   ¦       libimage_plugin.dll
¦   ¦   ¦   ¦       liblive555_plugin.dll
¦   ¦   ¦   ¦       libmjpeg_plugin.dll
¦   ¦   ¦   ¦       libmkv_plugin.dll
¦   ¦   ¦   ¦       libmod_plugin.dll
¦   ¦   ¦   ¦       libmp4_plugin.dll
¦   ¦   ¦   ¦       libmpc_plugin.dll
¦   ¦   ¦   ¦       libmpgv_plugin.dll
¦   ¦   ¦   ¦       libnsc_plugin.dll
¦   ¦   ¦   ¦       libnsv_plugin.dll
¦   ¦   ¦   ¦       libnuv_plugin.dll
¦   ¦   ¦   ¦       libogg_plugin.dll
¦   ¦   ¦   ¦       libplaylist_plugin.dll
¦   ¦   ¦   ¦       libps_plugin.dll
¦   ¦   ¦   ¦       libpva_plugin.dll
¦   ¦   ¦   ¦       librawaud_plugin.dll
¦   ¦   ¦   ¦       librawdv_plugin.dll
¦   ¦   ¦   ¦       librawvid_plugin.dll
¦   ¦   ¦   ¦       libreal_plugin.dll
¦   ¦   ¦   ¦       libsid_plugin.dll
¦   ¦   ¦   ¦       libsmf_plugin.dll
¦   ¦   ¦   ¦       libsubtitle_plugin.dll
¦   ¦   ¦   ¦       libts_plugin.dll
¦   ¦   ¦   ¦       libtta_plugin.dll
¦   ¦   ¦   ¦       libty_plugin.dll
¦   ¦   ¦   ¦       libvc1_plugin.dll
¦   ¦   ¦   ¦       libvobsub_plugin.dll
¦   ¦   ¦   ¦       libvoc_plugin.dll
¦   ¦   ¦   ¦       libwav_plugin.dll
¦   ¦   ¦   ¦       libxa_plugin.dll
¦   ¦   ¦   ¦
¦   ¦   ¦   +---packetizer
¦   ¦   ¦           libpacketizer_copy_plugin.dll
¦   ¦   ¦           libpacketizer_dirac_plugin.dll
¦   ¦   ¦           libpacketizer_flac_plugin.dll
¦   ¦   ¦           libpacketizer_h264_plugin.dll
¦   ¦   ¦           libpacketizer_mlp_plugin.dll
¦   ¦   ¦           libpacketizer_mpeg4audio_plugin.dll
¦   ¦   ¦           libpacketizer_mpeg4video_plugin.dll
¦   ¦   ¦           libpacketizer_mpegvideo_plugin.dll
¦   ¦   ¦           libpacketizer_vc1_plugin.dll
¦   ¦   ¦
¦   ¦   +---sqldrivers
¦   ¦   ¦       qsqlite.dll
¦   ¦   ¦
¦   ¦   +---translations
¦   ¦           qt_ar.qm
¦   ¦           qt_cs.qm
¦   ¦           qt_de.qm
¦   ¦           qt_es.qm
¦   ¦           qt_fr.qm
¦   ¦           qt_it.qm
¦   ¦           qt_ja.qm
¦   ¦           qt_ko.qm
¦   ¦           qt_pt.qm
¦   ¦           qt_ru.qm
¦   ¦           qt_zh_CN.qm
¦   ¦
¦   +---meta
¦           gpl_v3.txt
¦           installscript.qs
¦           package.xml
¦
+---org.miamplayer.plugins.coverfetcher
¦   +---data
¦   ¦       cover-fetcher.dll
¦   ¦
¦   +---meta
¦           package.xml
¦
+---org.miamplayer.plugins.minimode
¦   +---data
¦   ¦       mini-mode.dll
¦   ¦
¦   +---meta
¦           package.xml
¦
+---org.miamplayer.plugins.windowstoolbar
    +---data
    ¦       windows-toolbar.dll
    ¦
    +---meta
            package.xml