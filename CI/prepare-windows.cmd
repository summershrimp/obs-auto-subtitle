mkdir build64

cd build64
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0 -DQTDIR="%QTDIR64%" -Dw32-pthreads_DIR="%OBSPath%\build64\deps\w32-pthreads" -DW32_PTHREADS_LIB="%OBSPath%\build64\deps\w32-pthreads\%build_config%\w32-pthreads.lib" -DLibObs_DIR="%OBSPath%\build64\libobs" -DLIBOBS_INCLUDE_DIR="%OBSPath%\libobs" -DLIBOBS_LIB="%OBSPath%\build64\libobs\%build_config%\obs.lib" -DOBS_FRONTEND_LIB="%OBSPath%\build64\UI\obs-frontend-api\%build_config%\obs-frontend-api.lib" -DFFmpegPath="%DepsBasePath%" ..
