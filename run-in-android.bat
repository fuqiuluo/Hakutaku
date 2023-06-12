@echo off
cd cmake-build-androidarm64
adb push hak /data/local/tmp/
adb shell chmod a+x /data/local/tmp/hak
start cmd /k "adb shell su -c ./data/local/tmp/hak & pause&exit"

