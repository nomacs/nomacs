REM Be sure to run this in a VS Native Tools environment
REM "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

REM SET QT5="C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin"
SET QT5=%1
REM fix windows backslashed paths : /
SET QT5=%QT5:\=/%

SET MYPATH=%~dp0

call %mypath%/make-opencv.bat
call %mypath%/make-expat.bat
call %mypath%/make-exiv2.bat
REM call %mypath%/make-quazip.bat %qt5%
