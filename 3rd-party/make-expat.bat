REM Build expat

SET MYPATH=%~dp0
SET NAME=expat
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir% ( 
    echo %name% exists, skipping
    goto :eof
)

REM build expat which is needed for exiv2
cmake -DBUILD_examples=OFF -DBUILD_tests=OFF -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m
