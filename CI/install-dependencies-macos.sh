#!/bin/sh

OSTYPE=$(uname)
cd $(dirname $0)
cd ..

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-auto-subtitle - Error] macOS install dependencies script can be run on Darwin-type OS only."
    exit 1
fi

HAS_BREW=$(type brew 2>/dev/null)

if [ "${HAS_BREW}" = "" ]; then
    echo "[obs-auto-subtitle - Error] Please install Homebrew (https://www.brew.sh/) to build obs-auto-subtitle on macOS."
    exit 1
fi

# OBS Studio deps
echo "[obs-auto-subtitle] Updating Homebrew.."
brew update >/dev/null

echo "[obs-auto-subtitle] Checking installed Homebrew formulas.."
BREW_PACKAGES=$(brew list)
BREW_DEPENDENCIES="jack speexdsp ccache swig mbedtls"

for DEPENDENCY in ${BREW_DEPENDENCIES}; do
    if echo "${BREW_PACKAGES}" | grep -q "^${DEPENDENCY}\$"; then
        echo "[obs-auto-subtitle] Upgrading OBS-Studio dependency '${DEPENDENCY}'.."
        brew upgrade ${DEPENDENCY} 2>/dev/null
    else
        echo "[obs-auto-subtitle] Installing OBS-Studio dependency '${DEPENDENCY}'.."
        brew install ${DEPENDENCY} 2>/dev/null
    fi
done

# qt deps

install_qt_deps() {
    echo "[obs-ssp] Installing obs-ssp dependency 'QT ${1}'.."
    echo "Download..."
    curl --progress-bar -L -C - -O https://github.com/summershrimp/obs-deps/releases/download/${2}/macos-qt-${1}-x86_64-${2}.tar.gz
    echo "Unpack..."
    tar -xf ./macos-qt-${1}-x86_64-${2}.tar.gz -C /tmp
    xattr -r -d com.apple.quarantine /tmp/obsdeps
}

install_qt_deps 5.15.2 2021-04-03

install_obs_deps() {
    echo "Setting up pre-built macOS OBS dependencies v${1}"
    echo "Download..."
    curl --progress-bar -L -C - -O https://github.com/summershrimp/obs-deps/releases/download/${1}/macos-deps-x86_64-${1}.tar.gz
    echo "Unpack..."
    tar -xf ./macos-deps-x86_64-${1}.tar.gz -C /tmp
}

install_obs_deps 2021-04-03

# Fetch and install Packages app
# =!= NOTICE =!=
# Installs a LaunchDaemon under /Library/LaunchDaemons/fr.whitebox.packages.build.dispatcher.plist
# =!= NOTICE =!=

HAS_PACKAGES=$(type packagesbuild 2>/dev/null)

if [ "${HAS_PACKAGES}" = "" ]; then
    echo "[obs-auto-subtitle] Installing Packaging app (might require password due to 'sudo').."
    curl -L -o './Packages.pkg' --retry-connrefused -s --retry-delay 1 'https://github.com/summershrimp/obs-deps/releases/download/2021-04-03/Packages.pkg'
    sudo installer -pkg ./Packages.pkg -target /
fi
