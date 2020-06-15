REM build quazip
@echo off

SET MYPATH=%~dp0
REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%

SET QT_PATH=%1
SET NAME=quazip
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/Release/quazip5.dll ( 
    echo %name% exists, skipping
    goto :eof
)

echo building %name%
cmake -DCMAKE_PREFIX_PATH=%qt_path%^
 -DZLIB_INCLUDE_DIRS="%mypath%opencv/3rdparty/zlib"^
 -DZLIB_BUILD_PATH="%mypath%build/opencv/3rdparty"^
 -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release --target quazip5 -- -m
