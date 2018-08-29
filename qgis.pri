################################################
# DON'T EDIT THIS FILE, USE config.pri instead #
################################################

include( paths.pri )

!android {
  macx {
    LIBS += -F$${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/
    LIBS += -framework qgis_core

    INCLUDEPATH += $${QGIS_INSTALL_PATH}/QGIS.app/Contents/Frameworks/qgis_core.framework/Versions/Current/Headers
    INCLUDEPATH += $${GEOS_INCLUDE_PATH}
  }
  !macx {
    INCLUDEPATH += $${QGIS_INSTALL_PATH}/include/qgis
    LIBS += $${QGIS_INSTALL_PATH}/lib/libqgis_core.so
  }
}
android {
  QGIS_INSTALL_PATH = $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/files
  INCLUDEPATH += $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/include/qgis
  LIBS += $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libqgis_core.so

}

macx {
  DEFINES += "QGIS_PLUGIN_DIR=\\\"$$QGIS_INSTALL_PATH$$/QGIS.app/Contents/PlugIns/qgis\\\"" \
             "QGIS_INSTALL_DIR=\\\"$$QGIS_INSTALL_PATH$$\\\""
  QMAKE_RPATHDIR += $${QGIS_INSTALL_PATH}/Contents/MacOS/lib/
} else {
  DEFINES += "QGIS_PLUGIN_DIR=\\\"$$QGIS_INSTALL_PATH$$/lib/qgis/plugins/\\\"" \
             "QGIS_INSTALL_DIR=\\\"$$QGIS_INSTALL_PATH$$\\\""
  QMAKE_RPATHDIR += $${QGIS_INSTALL_PATH}/lib/
}

# Add QWT and QScintilla custom build paths if your distro only ships Qt4 versions
# and you had to build them manually for Qt5
QMAKE_RPATHDIR += $${QWT_DIR}
QMAKE_RPATHDIR += $${QSCINTILLA_DIR}
