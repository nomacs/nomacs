@echo off

if not defined NOMACS_DEPENDENCIES (
  echo Error NOMACS_DEPENDENCIES directory is not set
  exit /b 1
)

RMDIR /S /Q build-xz
mkdir build-xz
cd build-xz
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=%NOMACS_DEPENDENCIES% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DXZ_TOOL_XZDEC=OFF -DXZ_TOOL_LZMADEC=OFF -DXZ_TOOL_LZMAINFO=OFF -DXZ_TOOL_XZ=OFF -DXZ_DOC=OFF ../xz

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
