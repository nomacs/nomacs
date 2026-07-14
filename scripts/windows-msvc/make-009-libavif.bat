@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q libavif\ext\libyuv\build
mkdir libavif\ext\libyuv\build
cd libavif\ext\libyuv\build
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
if %ERRORLEVEL% NEQ 0 (
  echo Error configure libyuv
  exit /b 1
)

nmake yuv

if %ERRORLEVEL% NEQ 0 (
  echo Error Compile libyuv
  exit /b 1
)

cd ../../../..

RMDIR /S /Q build-libavif
mkdir build-libavif
cd build-libavif
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DAVIF_CODEC_AOM=SYSTEM -DAVIF_CODEC_DAV1D=SYSTEM -DAVIF_LIBYUV=LOCAL -DAVIF_LIBSHARPYUV=SYSTEM -D_LIBSHARPYUV_INCLUDEDIR=%NOMACS_DEPENDENCIES%/include/webp -DAVIF_CODEC_AOM_DECODE=OFF -DAVIF_CODEC_AOM_ENCODE=ON ../libavif

if %ERRORLEVEL% NEQ 0 (
  echo Error configure
  exit /b 1
)

nmake

if %ERRORLEVEL% NEQ 0 (
  echo Error Compile
  exit /b 1
)

nmake install

if %ERRORLEVEL% NEQ 0 (
  echo Error Install
  exit /b 1
)

cd ..
