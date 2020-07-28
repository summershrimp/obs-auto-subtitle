curl -o QWebsockets-ssl-win64.zip -kLO https://github.com/summershrimp/obs-auto-subtitle/releases/download/0.2.1/QWebsockets-ssl-win64.zip -f --retry 5 -C -
7z x QWebsockets-ssl-win64.zip -orelease\bin\64bit

mkdir package
cd package

git rev-parse --short HEAD > package-version.txt
set /p PackageVersion=<package-version.txt
del package-version.txt

REM Package ZIP archive
7z a "obs-auto-subtitle-%PackageVersion%-Windows.zip" "..\release\*"

REM Build installer
iscc ..\installer\installer.iss /O. /F"obs-auto-subtitle-%PackageVersion%-Windows-Installer"
