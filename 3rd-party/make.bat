REM Be sure to run this in a VS Native Tools environment
REM "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

SET QT_PATH="C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin"
SET MYPATH=%~dp0

REM call %mypath%/make-quazip.bat %qt_path%
call %mypath%/make-opencv.bat
call %mypath%/make-expat.bat
call %mypath%/make-exiv2.bat