REM build quazip

SET MYPATH=%~dp0
REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%

SET QT_PATH=%1
SET NAME=quazip
SET BUILD_DIR=%mypath%/build/%name%

echo %qt_path%

if exist %build_dir%/Release ( 
    echo %name% exists, skipping
    goto :eof
)

cmake -DCMAKE_PREFIX_PATH=%qt_path%^
 -DZLIB_INCLUDE_DIRS="%mypath%opencv/3rdparty/zlib"^
 -DZLIB_BUILD_PATH="%mypath%build/opencv/3rdparty"^
 -B%build_dir% %mypath%/%name%

msbuild -m -verbosity:minimal -target:quazip5 -property:Configuration=Debug  %build_dir%/%name%.sln
msbuild -m -verbosity:minimal -target:quazip5 -property:Configuration=Release  %build_dir%/%name%.sln
