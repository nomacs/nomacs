@echo off
set "qtver=%1"
set "prefix=Qt"
set "suffix=_src"
set "buildSuffix=%2"
set "folder=%prefix%%qtver%%suffix%"
if "%buildSuffix%" == "" set compilefolder=%prefix%%qtver%
if NOT "%buildSuffix%" == "" set compilefolder=%prefix%%qtver%_%buildSuffix%

if "%qtver%" == "" goto :leave

CALL git clone https://code.qt.io/qt/qt5.git %folder%
CALL cd %folder%
CALL git checkout .
CALL perl init-repository
::CALL git pull
CALL cd ..\

echo Download done... press enter to start compiling.
pause

set PATH=%cd%\%folder%\bin;%PATH%
set QTDIR=%cd%\%folder%\qtbase
mkdir %compilefolder%
call cd %compilefolder%
set QMAKESPEC=win32-msvc2015
set CL=/MP
CALL ..\%folder%\configure -confirm-license -debug-and-release -opensource -platform %QMAKESPEC% -opengl desktop -nomake examples -nomake tests -c++11 -mp -fontconfig -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -direct2d -shared

CALL nmake
::CALL nmake clean

:leave
if "%qtver%" == "" echo Please enter a version as the first parameter, and the build directory suffix as the second parameter
exit /b 1