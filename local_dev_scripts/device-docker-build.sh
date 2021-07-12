#!/usr/bin/env bash
source ./device-build.config

set -e
docker run -v $(pwd)/../:/usr/src/qfield -e "BUILD_FOLDER=${BUILD_FOLDER}" -e "ARCH=${ARCH}" -e "VERSION=${VERSION}" opengisch/qfield-sdk:${QFIELD_SDK_VERSION} /usr/src/qfield/scripts/docker-build.sh
