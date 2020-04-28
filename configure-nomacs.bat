REM configure nomacss

SET MYPATH=%~dp0
REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%

SET QT5=%1
SET NAME=nomacs
SET BUILD_DIR=%mypath%/build/%name%

cmake^
 -DENABLE_TRANSLATIONS=ON^
 -DCMAKE_PREFIX_PATH="%QT5%;%mypath%3rd-party/build/exiv2;%mypath%3rd-party/build/opencv;%mypath%3rd-party/build/quazip"^
 -DENABLE_RAW=OFF^
 -DENABLE_QUAZIP=OFF^
 -B%build_dir% %mypath%/ImageLounge

REM msbuild -m -verbosity:minimal -property:Configuration=Debug  %build_dir%/%name%.sln
REM msbuild -m -verbosity:minimal -property:Configuration=Release  %build_dir%/%name%.sln
