REM build opencv
@echo off

SET MYPATH=%~dp0
SET RELEASE=%1
SET NAME=opencv
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/bin/Release/ ( 
    echo %name% exists, skipping
    goto :eof
)

REM build opencv (that could take a while...)
echo building %name%

cmake^
 -DBUILD_PERF_TESTS=OFF^
 -DBUILD_TESTS=OFF^
 -DBUILD_opencv_java=OFF^
 -DBUILD_opencv_java_bindings_generator=OFF^
 -DBUILD_opencv_python=OFF^
 -DBUILD_opencv_apps=OFF^
 -DBUILD_opencv_dnn=OFF^
 -DBUILD_opencv_calib3d=OFF^
 -DBUILD_opencv_highgui=OFF^
 -DBUILD_opencv_photo=OFF^
 -DBUILD_opencv_python3=OFF^
 -DBUILD_opencv_python_tests=OFF^
 -DBUILD_opencv_python_bindings_generator=OFF^
 -DBUILD_opencv_stitiching=OFF^
 -DBUILD_opencv_video=OFF^
 -DBUILD_opencv_videoio=OFF^
 -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == release (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)
