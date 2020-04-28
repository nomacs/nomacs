REM build opencv

SET MYPATH=%~dp0
SET NAME=opencv
SET BUILD_DIR=%mypath%/build/%name%

if exist %build_dir% ( 
    echo %name% exists, skipping
    goto :eof
)

REM build opencv (that could take a while...)
cmake -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -B%build_dir% %mypath%/%name%

cmake --build %build_dir% --config Release -- -m
