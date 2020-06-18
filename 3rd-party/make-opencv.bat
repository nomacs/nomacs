REM build opencv
@echo off

SET MYPATH=%~dp0
SET RELEASE=%1
SET NAME=opencv
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/bin/Release/ ( 
    echo %name% exists, skipping
    goto :eof
)

REM build opencv (that could take a while...)
echo building %name%
cmake -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == release (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)