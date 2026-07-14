@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-pkgconf
mkdir build-pkgconf
cd build-pkgconf

meson setup --prefix=%NOMACS_DEPENDENCIES% --buildtype=release ../pkgconf

if %ERRORLEVEL% NEQ 0 (
  echo Error configure
  exit /b 1
)

ninja

if %ERRORLEVEL% NEQ 0 (
  echo Error Compile
  exit /b 1
)

ninja install

if %ERRORLEVEL% NEQ 0 (
  echo Error Install
  exit /b 1
)

cd ..
