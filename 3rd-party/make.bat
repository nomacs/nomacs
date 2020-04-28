echo building all nomacs dependencies - it's about tea time

@echo off

REM SET QT5="C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin"
SET QT5=%1
REM fix windows backslashed paths : /
SET QT5=%QT5:\=/%

SET MYPATH=%~dp0

call %mypath%/make-opencv.bat
call %mypath%/make-expat.bat
call %mypath%/make-exiv2.bat
call %mypath%/make-libraw.bat
call %mypath%/make-quazip.bat %qt5%
call %mypath%/imageformats/make.bat %qt5% %mypath%build/imageformats
