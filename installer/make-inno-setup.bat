echo off

set inno_local = "C:\Users\markus\AppData\Local\Programs\Inno Setup 6"
set inno_appveyor = "C:\Users\markus\AppData\Local\Programs\Inno Setup 6"
set sign = "C:\Program Files (x86)\Windows Kits\10\bin\x64\"

REM set your inno setup path
SET PATH=%PATH%;%inno_local%;%inno_appveyor%;%sign%;
SET ARCH=x64

call sign ./nomacs.%ARCH%/nomacs.exe %~1

REM compile
ISCC.exe ./nomacs-setup.iss

call sign ./nomacs-setup-%ARCH%.exe
