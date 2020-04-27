REM Build exiv2 - be sure to run build-expat.bat and build-opencv.bat first

SET MYPATH=%~dp0
REM fix windows backslashed paths : /
SET MYPATH=%MYPATH:\=/%
SET NAME=exiv2
SET BUILD_DIR=%mypath%/build/%name%

REM build exiv2
cmake^
 -DEXPAT_BUILD_PATH="%mypath%/build/expat"^
 -DEXPAT_INCLUDE_DIR="%mypath%/expat/lib"^
 -DZLIB_INCLUDE_DIR="%mypath%/opencv/3rdparty/zlib"^
 -DZLIB_BUILD_PATH="%mypath%/build/opencv/3rdparty"^
 -B%build_dir% %mypath%/%name%

msbuild -m -verbosity:minimal -property:Configuration=Debug  %build_dir%/%name%.sln
msbuild -m -verbosity:minimal -property:Configuration=Release  %build_dir%/%name%.sln
