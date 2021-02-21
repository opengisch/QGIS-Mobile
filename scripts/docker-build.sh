#!/bin/bash

# This is intended to be run within a opengisch/qfield-sdk docker container.
#
#     docker run -v $(pwd):/usr/src/qfield opengisch/qfield-sdk /usr/src/qfield/scripts/docker-build.sh
#
# The result will be put into `build-docker/android-build/build/outputs/apk/out-debug.apk`
#
#
# ANDROID_NDK_PLATFORM and QT_VERSION are defined in docker-qt-crystax
#
# APK_VERSION_CODE (MMmmFFNNA: major,minor,fix,number,architecture_index, e.g. 010203040 from v1.2.3-rc4 arm7) is required.
# APP_VERSION (e.g. v1.2.3, v1.2.3-rc4) is needed when a new version is published.
# APP_VERSION_STR ("v1.2.3 - Release Name", v1.2.3-rc4, PR1234, master-dev) is required. Human readable name.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
source ${DIR}/version_number.sh

set -e

if [[ -z $APK_VERSION_CODE ]]; then
  echo "*** ERROR MISSING APK_VERSION_CODE ENVIRONMENT VARIABLE"
  exit 2
fi

if [[ -z $APP_VERSION_STR ]]; then
  echo "*** ERROR MISSING APP_VERSION_STR"
  exit 2
fi

if [[ $( echo "${APK_VERSION_CODE} > 020000000" | bc ) == 1 ]]; then
  echo "*** ERROR TOO BIG VERSION CODE ${APK_VERSION_CODE}"
  echo "Remove this check if QField is now 2.x"
  exit 1
fi

SOURCE_DIR=/usr/src/qfield
if [[ -z ${BUILD_FOLDER+x} ]]; then
    BUILD_DIR=${SOURCE_DIR}/build-docker
else
    BUILD_DIR=${SOURCE_DIR}/${BUILD_FOLDER}
fi

# Set default values if missing
[[ -z ${ARCH} ]] && ARCH=armv7
[[ -z ${APP_NAME} ]] && APP_NAME="QField"
[[ -z ${APP_PACKAGE_NAME} ]] && APP_PACKAGE_NAME="qfield"

INSTALL_DIR=${BUILD_DIR}/out
QT_ANDROID=${QT_ANDROID_BASE}/android

echo "Package name ${APP_PACKAGE_NAME}"
echo "ANDROID_MINIMUM_PLATFORM: ${ANDROID_MINIMUM_PLATFORM}"
echo "ANDROID_TARGET_PLATFORM: ${ANDROID_TARGET_PLATFORM}"

if [[ -n ${APP_ICON} ]]; then
  # replace icon
  echo "Replacing icon with ${APP_ICON}"
  sed -i "s|<file alias=\"qfield_logo.svg\">icons/qfield_logo.svg</file>|<file alias=\"qfield_logo.svg\">icons/${APP_ICON}.svg</file>|" ${SOURCE_DIR}/images/images.qrc
  sed -i "s|@drawable/qfield_logo|@drawable/${APP_ICON}|g" ${SOURCE_DIR}/android/res/layout/unpacking_dialog.xml
  sed -i "s|@drawable/qfield_logo|@drawable/${APP_ICON}|g" ${SOURCE_DIR}/android/src/ch/opengis/qfield/QFieldProjectActivity.java
fi
if [[ "X${APP_PACKAGE_NAME}" != "Xqfield" ]]; then
  grep "ch.opengis.qfield" -l -r ${SOURCE_DIR}/android/ | xargs sed -i "s/ch.opengis.qfield/ch.opengis.${APP_PACKAGE_NAME}/g"
  grep "ch\.opengis\.qfield" -l -r ${SOURCE_DIR}/src/ | xargs sed -i "s/ch\.opengis\.qfield/ch.opengis.${APP_PACKAGE_NAME}/g"
  sed -i "s/ch_opengis_qfield/ch_opengis_${APP_PACKAGE_NAME//_/_1}/g" ${SOURCE_DIR}/src/core/androidplatformutilities.cpp
  mv ${SOURCE_DIR}/android/src/ch/opengis/qfield ${SOURCE_DIR}/android/src/ch/opengis/${APP_PACKAGE_NAME}
  sed -i "s|<string name=\"app_name\" translatable=\"false\">QField</string>|<string name=\"app_name\" translatable=\"false\">${APP_NAME}</string>|" ${SOURCE_DIR}/android/res/values/strings.xml
fi

mkdir -p ${BUILD_DIR}/.gradle
# androiddeployqt needs gradle and downloads it to /root/.gradle. By linking it to the build folder, this will be cached between builds.
ln -sfn ${BUILD_DIR}/.gradle $HOME/.gradle

pushd ${BUILD_DIR}

if [ "X${ARCH}" == "Xx86" ]; then
    export ANDROID_ARCH=x86
    export SHORTARCH=x86
elif [ "X${ARCH}" == "Xarmv7" ]; then
    export ANDROID_ARCH=armeabi-v7a
    export SHORTARCH=arm
elif [ "X${ARCH}" == "Xarm64_v8a" ]; then
    export ANDROID_ARCH=arm64-v8a
    export SHORTARCH=arm64
elif [ "X${ARCH}" == "Xx86_64" ]; then
    export ANDROID_ARCH=x86_64
    export SHORTARCH=x86_64
else
    echo "Error: Please report issue to enable support for arch (${ARCH})."
    exit 1
fi
export STAGE_PATH=/home/osgeo4a/${ANDROID_ARCH}

export ANDROID_CMAKE_LINKER_FLAGS="-fuse-ld=lld"

cmake \
	-G Ninja \
	-DAPK_VERSION_CODE=${APK_VERSION_CODE} \
	-DAPP_VERSION=${APP_VERSION} \
	-DAPP_VERSION_STR=${APP_VERSION_STR} \
	-DAPP_PACKAGE_NAME=${APP_PACKAGE_NAME} \
	-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
	-DQt5_DIR:PATH=${QT_ANDROID_BASE}/android/lib/cmake/Qt5 \
	-DANDROID_DEPLOY_QT=${QT_ANDROID_BASE}/android/bin/androiddeployqt \
	-DCMAKE_FIND_ROOT_PATH:PATH=${ANDROID_NDK_ROOT}\;${QT_ANDROID_BASE}/android/\;/home/osgeo4a/${ANDROID_ARCH} \
	-DANDROID_LINKER_FLAGS="${ANDROID_CMAKE_LINKER_FLAGS}" \
	-DANDROID_ABI=${ANDROID_ARCH} \
	-DANDROID_BUILD_ABI_${ANDROID_ARCH}=ON \
	-DQGIS_CORE_LIBRARY=/home/osgeo4a/${ANDROID_ARCH}/lib/libqgis_core_${ANDROID_ARCH}.so \
	-DQGIS_ANALYSIS_LIBRARY=/home/osgeo4a/${ANDROID_ARCH}/lib/libqgis_analysis_${ANDROID_ARCH}.so \
	-DQGIS_INCLUDE_DIR=/home/osgeo4a/${ANDROID_ARCH}/include/qgis/ \
	-DSQLITE3_INCLUDE_DIR:PATH=/home/osgeo4a/${ANDROID_ARCH}/include/ \
	-DOSGEO4A_STAGE_DIR:PATH=/home/osgeo4a/ \
	-DANDROID_SDK=${ANDROID_SDK_ROOT}/ \
	-DANDROID_NDK=${ANDROID_NDK_ROOT}/ \
	-DANDROID_STL:STRING=c++_shared \
	-DANDROID_PLATFORM=${ANDROID_MINIMUM_PLATFORM} \ `# This one is for NDK -> we need to link against the minimum supported version` \
	-DANDROID_TARGET_PLATFORM=${ANDROID_TARGET_PLATFORM} `# This one is for the APK, it ends up in the AndroidManifest.xml` \
	..

ninja

# Patch the input file for androiddeployqt with the build tools revision
# See https://forum.qt.io/topic/112578/unable-to-sign-android-app-wrong-path-for-zipalign
# Temporary workaround (fingers crossed)
cat <<< "$(jq ". += { \"sdkBuildToolsRevision\" : \"29.0.2\" }" < android_deployment_settings.json)" > android_deployment_settings_patched.json

if [ -n "${KEYNAME}" ] && [ -n "${KEYPASS}" ] && [ -n "${STOREPASS}" ]; then
    ${QT_ANDROID}/bin/androiddeployqt \
      --sign ${SOURCE_DIR}/keystore.p12 "${KEYNAME}" \
      --storepass "${STOREPASS}" \
      --keypass "${KEYPASS}" \
      --input ${BUILD_DIR}/android_deployment_settings_patched.json \
      --output ${BUILD_DIR}/android-build \
      --deployment bundled \
      --android-platform android-${ANDROID_TARGET_PLATFORM} \
      --gradle
else
    echo "-- Not signing the apk, KEYNAME, KEYPASS or STOREPASS is not set"
    ${QT_ANDROID}/bin/androiddeployqt \
      --input ${BUILD_DIR}/android_deployment_settings_patched.json \
      --output ${BUILD_DIR}/android-build \
      --deployment bundled \
      --android-platform android-${ANDROID_TARGET_PLATFORM} \
      --gradle
fi

cd android-build
./gradlew --no-daemon uploadCrashlyticsSymbolFileRelease

popd
