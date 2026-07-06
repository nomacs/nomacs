@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

if not defined QT6 (
  echo Error QT6 directory is not set
  exit /b 1
)

RMDIR /S /Q build-kimageformats
mkdir build-kimageformats
cd build-kimageformats
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH=%QT6% -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DKIMAGEFORMATS_HEIF=ON -DKIMAGEFORMATS_WITH_KNOWN_CRASHES_JXR=ON -DBUILD_TESTING=OFF ../kimageformats

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
