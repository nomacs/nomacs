@echo off
echo building all nomacs dependencies - it's about tea time

REM SET QT5="C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin"
SET QT5=%1
REM fix windows backslashed paths : /
SET QT5=%QT5:\=/%

REM if the script is called with "release" - we only build the release 
SET RELEASE=%1

SET MYPATH=%~dp0

call %mypath%/make-opencv.bat %release%
call %mypath%/make-expat.bat %release%
call %mypath%/make-exiv2.bat %release%
call %mypath%/make-libraw.bat %release%
call %mypath%/make-quazip.bat %qt5% %release%
call %mypath%/imageformats/make.bat %qt5% %mypath%build/imageformats

