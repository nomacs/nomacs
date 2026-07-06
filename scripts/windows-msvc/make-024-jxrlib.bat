@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-jxrlib
mkdir build-jxrlib
cd build-jxrlib
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ../jxrlib

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
