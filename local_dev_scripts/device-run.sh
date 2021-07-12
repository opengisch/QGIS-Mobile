#!/usr/bin/env bash

echo "logcat"
adb logcat -c
adb shell am force-stop ch.opengis.qfield 
adb shell am start -n ch.opengis.qfield/ch.opengis.qfield.QFieldActivity
sleep 3
adb logcat --pid=$(adb shell pidof -s ch.opengis.qfield)

