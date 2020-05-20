@echo off
REM configure nomacs

SET MYPATH=%~dp0

REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%
SET QT5=%QT5:\=/%

SET NAME=nomacs
SET BUILD_DIR=%mypath%build/%name%

echo "Qt Path:" %QT5%

cmake^
 -DCMAKE_PREFIX_PATH="%QT5%;%mypath%3rd-party/build/exiv2;%mypath%3rd-party/build/opencv;%mypath%3rd-party/build/quazip;%mypath%3rd-party/build/libraw;%mypath%3rd-party/build/imageformats/libheif;%mypath%3rd-party/build/imageformats/libde265"^
 -DENABLE_TRANSLATIONS=ON^
 -B%build_dir% %mypath%/ImageLounge

cmake --build %build_dir% --config Release -- -m
cmake --build %build_dir% --config Release --target INSTALL -- -m
