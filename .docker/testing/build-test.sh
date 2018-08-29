#!/bin/bash

set -e

mkdir /usr/src/build
cd /usr/src/build
cp /usr/src/config.pri.default /usr/src/config.pri
qmake CONFIG+=debug ..
make -j2
echo "travis_fold:end:build"
cd test
LD_LIBRARY_PATH=/usr/src/build/src/core:/usr/src/build/src/qgsquick xvfb-run ./test
