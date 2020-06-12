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

# qtndis deps
echo "[obs-auto-subtitle] Installing obs-auto-subtitle dependency 'QT 5.10.1'.."
# =!= NOTICE =!=
# When building QT5 from sources on macOS 10.13+, use local qt5 formula:
# brew install ./CI/macos/qt.rb
# Pouring from the bottle is much quicker though, so use bottle for now.
# =!= NOTICE =!=

brew install https://gist.githubusercontent.com/DDRBoxman/9c7a2b08933166f4b61ed9a44b242609/raw/ef4de6c587c6bd7f50210eccd5bd51ff08e6de13/qt.rb

# Pin this version of QT5 to avoid `brew upgrade`
# upgrading it to incompatible version
brew pin qt

# Fetch and install Packages app
# =!= NOTICE =!=
# Installs a LaunchDaemon under /Library/LaunchDaemons/fr.whitebox.packages.build.dispatcher.plist
# =!= NOTICE =!=

HAS_PACKAGES=$(type packagesbuild 2>/dev/null)

if [ "${HAS_PACKAGES}" = "" ]; then
    echo "[obs-auto-subtitle] Installing Packaging app (might require password due to 'sudo').."
    curl -o './Packages.pkg' --retry-connrefused -s --retry-delay 1 'https://s3-us-west-2.amazonaws.com/obs-nightly/Packages.pkg'
    sudo installer -pkg ./Packages.pkg -target /
fi
