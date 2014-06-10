@echo off
candle -ext WixUtilExtension MiamPlayer_x86.wxs
light -ext WixUtilExtension -ext WixUIExtension MiamPlayer_x86.wixobj