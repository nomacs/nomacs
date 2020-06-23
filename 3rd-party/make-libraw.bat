REM build libraw
@echo off

SET MYPATH=%~dp0
SET RELEASE=%1
SET NAME=libraw
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/Release/raw.dll ( 
    echo %name% exists, skipping
    goto :eof
)

REM build libraw
echo building %name%
cmake -DENABLE_EXAMPLES=OFF -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == release (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)
