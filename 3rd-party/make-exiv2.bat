@echo off
REM Build exiv2 - be sure to run build-expat.bat and build-opencv.bat first

SET MYPATH=%~dp0
SET RELEASE=%1
REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%
SET NAME=exiv2
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/Release/bin/exiv2.dll ( 
    echo %name% exists, skipping
    goto :eof
)

REM build exiv2
echo building %name%
cmake^
 -DEXPAT_BUILD_PATH="%mypath%/build/expat"^
 -DEXPAT_INCLUDE_DIR="%mypath%/expat/lib"^
 -DZLIB_INCLUDE_DIR="%mypath%/opencv/3rdparty/zlib"^
 -DZLIB_BUILD_PATH="%mypath%/build/opencv/3rdparty"^
 -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == release (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)