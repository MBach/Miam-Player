@echo off
candle -ext WixUtilExtension MmeMiamMiamMusicPlayer.wxs -arch x64
light -ext WixUtilExtension -ext WixUIExtension MmeMiamMiamMusicPlayer.wixobj