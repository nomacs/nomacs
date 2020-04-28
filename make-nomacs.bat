REM configure nomacs

SET MYPATH=%~dp0
SET QT5=%1

REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%
SET QT5=%QT5:\=/%

SET NAME=nomacs
SET BUILD_DIR=%mypath%build/%name%

echo %QT5%

cmake^
 -DENABLE_TRANSLATIONS=OFF^
 -DCMAKE_PREFIX_PATH="%QT5%;%mypath%3rd-party/build/exiv2;%mypath%3rd-party/build/opencv;%mypath%3rd-party/build/quazip;%mypath%3rd-party/build/libraw;"^
 -B%build_dir% %mypath%/ImageLounge

cmake --build %build_dir% --config Release -- -m
cmake --build %build_dir% --config Release --target INSTALL -- -m


REM cmake --build %build_dir% -- -m

REM msbuild -m -verbosity:minimal -property:Configuration=Debug  %build_dir%/%name%.sln
REM msbuild -m -verbosity:minimal -property:Configuration=Release  %build_dir%/%name%.sln
