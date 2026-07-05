@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-dav1d
mkdir build-dav1d
cd build-dav1d

meson setup --prefix=%NOMACS_DEPENDENCIES% --default-library=shared --buildtype=release -Denable_tools=false -Denable_tests=false ../dav1d

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
