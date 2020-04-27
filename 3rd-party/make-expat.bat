REM Build expat

SET MYPATH=%~dp0
SET NAME=expat
SET BUILD_DIR=%mypath%/%name%/build

REM build expat which is needed for exiv2
cmake -DBUILD_examples=OFF -DBUILD_tests=OFF -B%build_dir% %mypath%/%name%

msbuild -m -verbosity:minimal -property:Configuration=Debug %build_dir%/%name%.sln
msbuild -m -verbosity:minimal -property:Configuration=Release %build_dir%/%name%.sln
