@echo off
SETLOCAL

::android ndk path set here
set MINGW_DIR=C:/mingw32/bin

mkdir build
cd build
mkdir mingw
cd mingw

cmake -G"Ninja"^
 -DCMAKE_AR="%MINGW_DIR%/ar.exe"^
 -DCMAKE_C_COMPILER="%MINGW_DIR%/gcc.exe"^
 -DCMAKE_CXX_COMPILER="%MINGW_DIR%/g++.exe"^
 -DCMAKE_RC_COMPILER="%MINGW_DIR%/windres.exe"^
 ../..

pause
ninja

del /F CMakeCache.txt
cd ..
pause
