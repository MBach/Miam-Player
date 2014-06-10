@echo off
candle -ext WixUtilExtension MiamPlayer_x64.wxs -arch x64
light -ext WixUtilExtension -ext WixUIExtension MiamPlayer_x64.wixobj