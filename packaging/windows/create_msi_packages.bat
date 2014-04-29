@echo off
candle -ext WixUtilExtension MiamPlayer_x64.wxs -arch x64
light -ext WixUtilExtension -ext WixUIExtension MiamPlayer_x64.wixobj
candle -ext WixUtilExtension MiamPlayer_x86.wxs
light -ext WixUtilExtension -ext WixUIExtension MiamPlayer_x86.wixobj