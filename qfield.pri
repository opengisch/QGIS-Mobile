################################################
# DON'T EDIT THIS FILE, USE config.pri instead #
################################################

include( paths.pri )

QMAKE_CXXFLAGS += -std=c++11

android {
  LIBS += -L$${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib
  INCLUDEPATH += $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/include \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/include/qgis

  ANDROID_EXTRA_LIBS = \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libcrystax.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libexpat.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgeos.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgeos_c.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgslcblas.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libsqlite3.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libcharset.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libiconv.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libfreexl.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libtiff.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgdal.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libproj.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libspatialindex.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libpq.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libspatialite.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libqca-qt5.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libqgis_core.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libqgis_native.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libqt5keychain.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libzip.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libspatialiteprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libdelimitedtextprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgdalprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libgpxprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libmssqlprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libogrprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libowsprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libpostgresprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libspatialiteprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libwcsprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libwfsprovider.so \
    $${OSGEO4A_STAGE_DIR}/$$ANDROID_TARGET_ARCH$$/lib/libwmsprovider.so \
    $$QT_LIBS_DIR/libQt5OpenGL.so \
    $$QT_LIBS_DIR/libQt5PrintSupport.so \
    $$QT_LIBS_DIR/libQt5Concurrent.so \
    $$QT_LIBS_DIR/libQt5Sensors.so \
    $$QT_LIBS_DIR/libQt5Sql.so \
    $$QT_LIBS_DIR/libQt5Svg.so \
    $$QT_LIBS_DIR/libQt5SerialPort.so \
    $$QT_LIBS_DIR/libQt5PrintSupport.so
}

