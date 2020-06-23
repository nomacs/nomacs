REM Build expat
@echo off

SET MYPATH=%~dp0
SET RELEASE=%1
SET NAME=expat
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir%/Release/expat.dll ( 
    echo %name% exists, skipping
    goto :eof
)

REM build expat which is needed for exiv2
echo building %name%
cmake -DBUILD_examples=OFF -DBUILD_tests=OFF -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m

if not %RELEASE% == release (
    echo building %name% - [Debug]
    cmake --build %build_dir% --config Debug -- -m
)
