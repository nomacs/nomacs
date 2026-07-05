@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

IF NOT EXIST c:\msys64\usr\bin\bash.exe (
  echo Msys2 not found
  exit /b 1
)

set MSYS2_PATH_TYPE=inherit
set CHERE_INVOKING=1
c:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu"
c:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -S make"

cd x264
c:\msys64\usr\bin\bash.exe -lc "CC=cl ./configure --prefix="%NOMACS_DEPENDENCIES%" --disable-cli  --enable-static --disable-bashcompletion --disable-opencl --disable-interlaced  --bit-depth=8 --chroma-format=420 --disable-avs --disable-swscale --disable-lavf --disable-ffms  --disable-gpac --disable-lsmash"

if %ERRORLEVEL% NEQ 0 (
  echo Error configure
  exit /b 1
)

c:\msys64\usr\bin\bash -lc "make"

if %ERRORLEVEL% NEQ 0 (
  echo Error Compile
  exit /b 1
)

c:\msys64\usr\bin\bash -lc "make install"

if %ERRORLEVEL% NEQ 0 (
  echo Error Install
  exit /b 1
)

cd ..
