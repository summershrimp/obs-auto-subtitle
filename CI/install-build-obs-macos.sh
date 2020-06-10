#!/bin/sh

cd $(dirname $0)
cd ../..

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-auto-subtitle - Error] macOS obs-studio build script can be run on Darwin-type OS only."
    exit 1
fi

HAS_CMAKE=$(type cmake 2>/dev/null)
HAS_GIT=$(type git 2>/dev/null)

if [ "${HAS_CMAKE}" = "" ]; then
    echo "[obs-auto-subtitle - Error] CMake not installed - please run 'install-dependencies-macos.sh' first."
    exit 1
fi

if [ "${HAS_GIT}" = "" ]; then
    echo "[obs-auto-subtitle - Error] Git not installed - please install Xcode developer tools or via Homebrew."
    exit 1
fi
if [ ! -d obsdeps ]; then
    echo "[obs-auto-subtitle] Downloading and unpacking OBS dependencies"
    wget -c --retry-connrefused --waitretry=1 https://obs-nightly.s3.amazonaws.com/osx-deps-2018-08-09.tar.gz
    tar -xf ./osx-deps-2018-08-09.tar.gz -C .
fi

# Build obs-studio
if [ ! -d obs-studio/.git ]; then
    echo "[obs-auto-subtitle] Cloning obs-studio from GitHub.."
    rm -fr obs-studio
    git clone https://github.com/obsproject/obs-studio
    cd obs-studio
else
    echo "[obs-auto-subtitle] Fetching obs-studio.."
    cd obs-studio
    git fetch
fi

OBSLatestTag=$(git describe --tags --abbrev=0)
git checkout $OBSLatestTag
rm -fr build
mkdir -p build && cd build
echo "[obs-auto-subtitle] Building obs-studio.."
cmake .. \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
    -DDISABLE_PLUGINS=true \
    -DENABLE_SCRIPTING=0 \
    -DDepsPath=$(pwd)/../../obsdeps \
    -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake \
&& make -j4
