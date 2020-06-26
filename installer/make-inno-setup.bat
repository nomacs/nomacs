echo off
REM set your inno setup path
SET PATH=%PATH%;"C:\Users\markus\AppData\Local\Programs\Inno Setup 6";"C:\Program Files (x86)\Windows Kits\10\bin\x64\";
SET ARCH=x64

call sign ./nomacs.%ARCH%/nomacs.exe %~1

REM compile
ISCC.exe ./nomacs-setup.iss

call sign ./nomacs-setup-%ARCH%.exe
