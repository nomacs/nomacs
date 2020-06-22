@echo off
REM configure nomacs
REM "C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin"

SET MYPATH=%~dp0
SET QT5=%1
SET RELEASE="%2"

REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%
SET QT5=%QT5:\=/%

SET NAME=nomacs
SET BUILD_DIR=%mypath%build/%name%

echo "Qt Path:" %QT5%

cmake^
 -DCMAKE_PREFIX_PATH="%QT5%;%mypath%3rd-party/build/exiv2;%mypath%3rd-party/build/opencv;%mypath%3rd-party/build/quazip;%mypath%3rd-party/build/libraw;%mypath%3rd-party/build/imageformats/libheif;%mypath%3rd-party/build/imageformats/libde265"^
 -DENABLE_TRANSLATIONS=ON^
 -DENABLE_HEIF=ON^
 -B%build_dir% %mypath%/ImageLounge

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == "release" (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)

cmake --build %build_dir% --config Release --target INSTALL -- -m
