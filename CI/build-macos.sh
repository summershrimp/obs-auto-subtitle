#!/bin/sh

cd $(dirname $0)
cd ..

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-auto-subtitle - Error] macOS build script can be run on Darwin-type OS only."
    exit 1
fi

HAS_CMAKE=$(type cmake 2>/dev/null)

if [ "${HAS_CMAKE}" = "" ]; then
    echo "[obs-auto-subtitle - Error] CMake not installed - please run 'install-dependencies-macos.sh' first."
    exit 1
fi

#export QT_PREFIX="$(find /usr/local/Cellar/qt5 -d 1 | tail -n 1)"

echo "[obs-auto-subtitle] Building 'obs-auto-subtitle' for macOS."
rm -fr build
mkdir -p build && cd build
cmake .. \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
    -DQTDIR="/tmp/obsdeps" \
    -DLIBOBS_INCLUDE_DIR=../../obs-studio/libobs \
    -DLIBOBS_LIB=../../obs-studio/libobs \
    -DOBS_FRONTEND_LIB="$(pwd)/../../obs-studio/build/UI/obs-frontend-api/libobs-frontend-api.dylib" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DFFmpegPath="/tmp/obsdeps" \
&& make -j4
