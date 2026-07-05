@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-openh264
mkdir build-openh264
cd build-openh264

meson setup --prefix=%NOMACS_DEPENDENCIES% --default-library=shared --buildtype=release -Dtests=disabled ../openh264

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
