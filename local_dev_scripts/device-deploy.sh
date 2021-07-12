#!/usr/bin/env bash
source ./device-build.config

set +e
echo "uninstalling ..."
adb uninstall ch.opengis.qfield

set -e
echo "installing..."
adb install ../$BUILD_FOLDER/out/build/outputs/apk/debug/out-debug.apk

