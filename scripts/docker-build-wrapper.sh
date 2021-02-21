#!/bin/env bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )

USER_ID=$(stat -c '%u' "${BASH_SOURCE[0]}")
GROUP_ID=$(stat -c '%g' "${BASH_SOURCE[0]}")
USER_NAME=$(stat -c '%U' "${BASH_SOURCE[0]}")

groupadd -g "$GROUP_ID" qfield
useradd --home-dir /io/data --shell /bin/bash --uid "$USER_ID" --gid "$GROUP_ID" "$USER_NAME"
mkdir -p /io/data
chown $USER_ID:$GROUP_ID /io/data
chown $USER_ID:$GROUP_ID /opt/android-sdk
chown $USER_ID:$GROUP_ID /opt/android-sdk/*

su "$USER_NAME" -c ${DIR}/docker-build.sh
