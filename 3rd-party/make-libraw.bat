REM build libraw
@echo off

SET MYPATH=%~dp0
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