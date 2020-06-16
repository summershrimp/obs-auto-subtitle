#!/bin/bash

set -e
cd $(dirname $0)
cd ..

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-auto-subtitle - Error] macOS package script can be run on Darwin-type OS only."
    exit 1
fi

echo "[obs-auto-subtitle] Preparing package build"
export QT_CELLAR_PREFIX="$(/usr/bin/find /usr/local/Cellar/qt -d 1 | sort -t '.' -k 1,1n -k 2,2n -k 3,3n | tail -n 1)"

GIT_HASH=$(git rev-parse --short HEAD)
GIT_BRANCH_OR_TAG=$(git name-rev --name-only HEAD | awk -F/ '{print $NF}')

VERSION="$GIT_HASH-$GIT_BRANCH_OR_TAG"

FILENAME_UNSIGNED="obs-auto-subtitle-$VERSION-Unsigned.pkg"
FILENAME="obs-auto-subtitle-$VERSION.pkg"

echo "[obs-auto-subtitle] Modifying obs-auto-subtitle.so"
install_name_tool \
	-change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets \
		@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
	-change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
	-change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
	-change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork \
		@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork \
	-change /usr/local/opt/qt/lib/QtWebSockets.framework/Versions/5/QtWebSockets \
		@executable_path/../Frameworks/QtWebSockets.framework/Versions/5/QtWebSockets \
	./build/obs-auto-subtitle.so

# Check if replacement worked
echo "[obs-auto-subtitle] Dependencies for obs-auto-subtitle"
otool -L ./build/obs-auto-subtitle.so

echo "[obs-auto-subtitle] Copy QtWebSockets.framework to here"
rm -fr ./build/QtWebSockets.framework
cp -a /usr/local/opt/qt/lib/QtWebSockets.framework ./build/
chmod -R +w ./build/QtWebSockets.framework

echo "[obs-auto-subtitle] Modifying QtWebSockets.framework"
install_name_tool \
  -id @executable_path/../Frameworks/QtWebSockets.framework/Versions/5/QtWebSockets \
  ./build/QtWebSockets.framework/Versions/5/QtWebSockets

install_name_tool \
  -change /usr/local/Cellar/qt/5.14.1/lib/QtCore.framework/Versions/5/QtCore \
  @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
  -change /usr/local/Cellar/qt/5.14.1/lib/QtNetwork.framework/Versions/5/QtNetwork \
  @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork \
  ./build/QtWebSockets.framework/Versions/5/QtWebSockets

# Check if replacement worked
echo "[obs-auto-subtitle] Dependencies for QtWebSockets.framework"
otool -L ./build/QtWebSockets.framework/Versions/5/QtWebSockets


if [[ "$RELEASE_MODE" == "True" ]]; then
	echo "[obs-auto-subtitle] Signing plugin binary: obs-auto-subtitle.so"
	codesign --sign "$CODE_SIGNING_IDENTITY" ./build/obs-auto-subtitle.so
else
	echo "[obs-auto-subtitle] Skipped plugin codesigning"
fi

echo "[obs-auto-subtitle] Actual package build"
packagesbuild ./installer/obs-auto-subtitle.pkgproj

echo "[obs-auto-subtitle] Renaming obs-auto-subtitle.pkg to $FILENAME"
mkdir -p release
mv ./installer/build/obs-auto-subtitle.pkg ./release/$FILENAME_UNSIGNED

if [[ "$RELEASE_MODE" == "True" ]]; then
	echo "[obs-auto-subtitle] Signing installer: $FILENAME"
	productsign \
		--sign "$INSTALLER_SIGNING_IDENTITY" \
		./release/$FILENAME_UNSIGNED \
		./release/$FILENAME
	rm ./release/$FILENAME_UNSIGNED

	echo "[obs-auto-subtitle] Submitting installer $FILENAME for notarization"
	zip -r ./release/$FILENAME.zip ./release/$FILENAME
	UPLOAD_RESULT=$(xcrun altool \
		--notarize-app \
		--primary-bundle-id "fr.palakis.obs-auto-subtitle" \
		--username "$AC_USERNAME" \
		--password "$AC_PASSWORD" \
		--asc-provider "$AC_PROVIDER_SHORTNAME" \
		--file "./release/$FILENAME.zip")
	rm ./release/$FILENAME.zip

	REQUEST_UUID=$(echo $UPLOAD_RESULT | awk -F ' = ' '/RequestUUID/ {print $2}')
	echo "Request UUID: $REQUEST_UUID"

	echo "[obs-auto-subtitle] Wait for notarization result"
	# Pieces of code borrowed from rednoah/notarized-app
	while sleep 30 && date; do
		CHECK_RESULT=$(xcrun altool \
			--notarization-info "$REQUEST_UUID" \
			--username "$AC_USERNAME" \
			--password "$AC_PASSWORD" \
			--asc-provider "$AC_PROVIDER_SHORTNAME")
		echo $CHECK_RESULT

		if ! grep -q "Status: in progress" <<< "$CHECK_RESULT"; then
			echo "[obs-auto-subtitle] Staple ticket to installer: $FILENAME"
			xcrun stapler staple ./release/$FILENAME
			break
		fi
	done
else
	echo "[obs-auto-subtitle] Skipped installer codesigning and notarization"
fi
