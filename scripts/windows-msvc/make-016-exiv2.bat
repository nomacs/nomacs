@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-exiv2
mkdir build-exiv2
cd build-exiv2
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DEXIV2_BUILD_EXIV2_COMMAND=OFF -DEXIV2_ENABLE_INIH=OFF ../exiv2

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
