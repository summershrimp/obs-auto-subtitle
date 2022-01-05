set build_config=RelWithDebInfo
set DepsBasePath=D:\Projects\obsdep
set DepsPath32=%DepsBasePath%\win32
set DepsPath64=%DepsBasePath%\win64
set QtBaseDir=E:\Qt
set QTDIR32=%QtBaseDir%\5.15.2\msvc2019
set QTDIR64=%QtBaseDir%\5.15.2\msvc2019_64
set OBSPath=D:\Projects\obs-studio
set https_proxy=http://localhost:10809
set OBSTargetVersion=26.1.0

call .\CI\download-obs-deps.cmd

call .\CI\checkout-cmake-obs-windows.cmd

cmake --build "%OBSPath%\build64" --config %build_config%

call .\CI\prepare-windows.cmd
cmake --build . --config %build_config%
