@echo off
cd cmake-build-androidarm64
adb push Hakutaku /data/local/tmp/
adb shell chmod a+x /data/local/tmp/Hakutaku
start cmd /k "adb shell ./data/local/tmp/Hakutaku & pause&exit"

