echo off

SET inno_local="%LOCALAPPDATA%\Programs\Inno Setup 6"
SET inno_appveyor="C:\Program Files (x86)\Inno Setup 6"
SET sign="C:\Program Files (x86)\Windows Kits\10\bin\x64\"

REM set your inno setup path
SET PATH=%PATH%;%inno_local%;%inno_appveyor%;%sign%;
SET ARCH=x64

call sign ./nomacs.%ARCH%/nomacs.exe %~1

REM compile
ISCC.exe ./nomacs-setup.iss

call sign ./nomacs-setup-%ARCH%.exe
