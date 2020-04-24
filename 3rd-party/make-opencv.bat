REM build opencv

SET MYPATH=%~dp0
SET NAME=opencv
SET BUILD_DIR=%mypath%/%name%/build

REM build opencv (that could take a while...)
cmake -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -B%build_dir% %mypath%/%name%

msbuild -property:Configuration=Debug %build_dir%/%name%.sln
msbuild -property:Configuration=Release %build_dir%/%name%.sln
