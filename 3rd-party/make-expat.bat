REM Build expat

SET NAME=expat
SET BUILD_DIR=./%name%/build

REM build expat which is needed for exiv2
cmake -DBUILD_examples=OFF -DBUILD_tests=OFF -B%build_dir% ./%name%

msbuild -property:Configuration=Debug %build_dir%/%name%.sln
msbuild -property:Configuration=Release %build_dir%/%name%.sln
