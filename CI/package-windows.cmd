copy %QTDIR64%\bin\Qt5Network.dll  release\bin\64bit\Qt5Network.dll
copy %QTDIR64%\bin\Qt5WebSockets.dll  release\bin\64bit\Qt5WebSockets.dll
copy %QtBaseDir%\Tools\OpenSSL\Win_x64\bin\libcrypto-1_1-x64.dll release\bin\64bit\libcrypto-1_1-x64.dll
copy %QtBaseDir%\Tools\OpenSSL\Win_x64\bin\libssl-1_1-x64.dll release\bin\64bit\libssl-1_1-x64.dll

mkdir package
cd package

git rev-parse --short HEAD > package-version.txt
set /p PackageVersion=<package-version.txt
del package-version.txt

REM Package ZIP archive
7z a "obs-auto-subtitle-%PackageVersion%-Windows.zip" "..\release\*"

REM Build installer
iscc ..\installer\installer.iss /O. /F"obs-auto-subtitle-%PackageVersion%-Windows-Installer"
