@echo off
candle -ext WixUtilExtension MiamPlayer.wxs -arch x64
light -ext WixUtilExtension -ext WixUIExtension MiamPlayer.wixobj