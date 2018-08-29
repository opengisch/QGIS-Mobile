#!/usr/bin/env bash

# In build environment set QGIS_INSTALL_PATH similarly as defined in config.pri
# Then, in project config, in run, add a custom deployment step
#   command: this script
#   arguments: "${QGIS_INSTALL_PATH}"

QGIS_INSTALL_PATH=$1

install_name_tool -change @executable_path/../Frameworks/qgis_core.framework/qgis_core ${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/qgis_core.framework/qgis_core src/qgsquick/libqgsquick.dylib
install_name_tool -change @executable_path/../Frameworks/qgis_core.framework/qgis_core ${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/qgis_core.framework/qgis_core src/core/libcore.dylib
install_name_tool -change @executable_path/../Frameworks/qgis_core.framework/qgis_core ${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/qgis_core.framework/qgis_core src/app/qfield.app/Contents/MacOS/qfield
install_name_tool -change @executable_path/../Frameworks/qgis_core.framework/qgis_core ${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/qgis_core.framework/qgis_core test/test.app/Contents/MacOS/test
