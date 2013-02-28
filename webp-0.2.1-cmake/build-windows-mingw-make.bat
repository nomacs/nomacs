@echo off
SETLOCAL

::android ndk path set here
set MINGW_DIR=C:/mingw32/bin

mkdir build
cd build
mkdir mingw
cd mingw

cmake -G"MinGW Makefiles"^
 -DCMAKE_MAKE_PROGRAM="%MINGW_DIR%/mingw32-make.exe"^
 -DCMAKE_C_COMPILER="%MINGW_DIR%/i686-w64-mingw32-gcc.exe"^
 -DCMAKE_CXX_COMPILER="%MINGW_DIR%/i686-w64-mingw32-g++.exe"^
 -DCMAKE_RC_COMPILER="%MINGW_DIR%/i686-w64-mingw32-windres.exe"^
 ../..

pause
"%MINGW_DIR%/mingw32-make.exe"

del /F CMakeCache.txt
cd ..
pause
